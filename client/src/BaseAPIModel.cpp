#include "BaseAPIModel.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcessEnvironment>
#include <QScopeGuard>

namespace {
constexpr auto kPathProperty = "truten_path";
constexpr auto kMethodProperty = "truten_method";
constexpr auto kBodyProperty = "truten_body";
constexpr auto kWithTokenProperty = "truten_with_token";
constexpr auto kRetryCountProperty = "truten_retry_count";
}

QString BaseModel::m_accessToken = QString();
QString BaseModel::m_refreshToken = QString();
QString BaseModel::m_userId = QString();
bool BaseModel::m_isAdmin = false;

QString BaseModel::baseUrl() {
    const QString overrideUrl = qEnvironmentVariable("TRUTEN_API_BASE_URL");
    if (!overrideUrl.isEmpty()) {
        return overrideUrl;
    }
    return QStringLiteral("http://127.0.0.1:8080/v1");
}

BaseModel::BaseModel(QNetworkAccessManager *manager, QObject *parent)
    : QObject(parent), m_manager(manager) {
}

void BaseModel::clearSession() {
    m_accessToken.clear();
    m_refreshToken.clear();
    m_userId.clear();
    m_isAdmin = false;
}

QNetworkReply *BaseModel::sendRequest(
    const QByteArray &method,
    const QString &path,
    const QByteArray &body,
    Token withToken,
    int retryCount
) {
    QNetworkRequest request(QUrl(baseUrl() + path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (withToken == Token::WITH_TOKEN) {
        request.setRawHeader(
            "Authorization", "Bearer " + m_accessToken.toUtf8()
        );
    }

    QNetworkReply *reply = nullptr;
    if (method == "GET") {
        reply = m_manager->get(request);
    } else if (method == "POST") {
        reply = m_manager->post(request, body);
    } else {
        reply = m_manager->sendCustomRequest(request, method, body);
    }

    reply->setProperty(kPathProperty, path);
    reply->setProperty(kMethodProperty, method);
    reply->setProperty(kBodyProperty, body);
    reply->setProperty(
        kWithTokenProperty, withToken == Token::WITH_TOKEN
    );
    reply->setProperty(kRetryCountProperty, retryCount);

    return reply;
}

QNetworkReply *BaseModel::sendGetRequest(const QString &path, Token withToken) {
    return sendRequest("GET", path, QByteArray(), withToken);
}

QNetworkReply *BaseModel::sendPostRequest(
    const QString &path,
    const QJsonObject &json,
    Token withToken
) {
    return sendRequest(
        "POST", path, QJsonDocument(json).toJson(QJsonDocument::Compact),
        withToken
    );
}

QNetworkReply *BaseModel::sendDeleteRequest(
    const QString &path,
    const QJsonObject &json,
    Token withToken
) {
    return sendRequest(
        "DELETE", path, QJsonDocument(json).toJson(QJsonDocument::Compact),
        withToken
    );
}

QString BaseModel::handleReplyError(QNetworkReply *reply) {
    QString userMessage;
    qDebug() << "Reply error: " << reply->errorString() << '\n';

    switch (reply->error()) {
        case QNetworkReply::ConnectionRefusedError:
            userMessage = "Сервер временно недоступен. Попробуйте позже.";
            break;
        case QNetworkReply::ContentNotFoundError:  // 404
            userMessage = "Запрошенный ресурс не найден (404).";
            break;
        case QNetworkReply::AuthenticationRequiredError:  // 401
            userMessage = "Ошибка авторизации. Пожалуйста, войдите снова.";
            break;
        case QNetworkReply::ContentAccessDenied:  // 403
            userMessage = "У вас нет прав для этого действия.";
            break;
        case QNetworkReply::TimeoutError:
            userMessage = "Время ожидания истекло.";
            break;
        default:
            userMessage =
                "Произошла непредвиденная ошибка: " + reply->errorString();
    }

    return userMessage;
}

void BaseModel::refreshAndRetry(
    const QString &path,
    const QByteArray &method,
    const QByteArray &body,
    int retryCount,
    std::function<void(const QJsonObject &)> onSuccess,
    std::function<void(const QString &)> onError
) {
    if (m_refreshToken.isEmpty()) {
        clearSession();
        onError("Ошибка авторизации. Пожалуйста, войдите снова.");
        return;
    }

    QJsonObject refreshJson;
    refreshJson["refreshToken"] = m_refreshToken;
    QNetworkReply *refreshReply =
        sendPostRequest("/auth/refresh", refreshJson, Token::WITHOUT_TOKEN);

    connect(
        refreshReply, &QNetworkReply::finished, this,
        [this, refreshReply, path, method, body, retryCount,
         onSuccess = std::move(onSuccess), onError = std::move(onError)]() mutable {
            auto deleteRefresh =
                qScopeGuard([refreshReply] { refreshReply->deleteLater(); });

            const int statusCode = refreshReply
                                       ->attribute(
                                           QNetworkRequest::HttpStatusCodeAttribute
                                       )
                                       .toInt();

            if (refreshReply->error() != QNetworkReply::NoError ||
                statusCode < 200 || statusCode >= 300) {
                clearSession();
                onError("Ошибка авторизации. Пожалуйста, войдите снова.");
                return;
            }

            const QByteArray rawData = refreshReply->readAll();
            QJsonParseError parseError;
            const QJsonDocument doc =
                QJsonDocument::fromJson(rawData, &parseError);
            const QString newAccess = doc.object()["accessToken"].toString();

            if (parseError.error != QJsonParseError::NoError ||
                newAccess.isEmpty()) {
                clearSession();
                onError("Ошибка авторизации. Пожалуйста, войдите снова.");
                return;
            }

            setAccessToken(newAccess);

            QNetworkReply *retryReply = sendRequest(
                method, path, body, Token::WITH_TOKEN, retryCount + 1
            );
            connect(
                retryReply, &QNetworkReply::finished, this,
                [this, retryReply, onSuccess = std::move(onSuccess),
                 onError = std::move(onError)]() mutable {
                    handleReply(
                        retryReply, std::move(onSuccess), std::move(onError)
                    );
                }
            );
        }
    );
}

void BaseModel::handleReply(
    QNetworkReply *reply,
    std::function<void(const QJsonObject &)> onSuccess,
    std::function<void(const QString &)> onError
) {
    auto deleteReply = qScopeGuard([reply] { reply->deleteLater(); });

    const int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const bool withToken = reply->property(kWithTokenProperty).toBool();
    const int retryCount = reply->property(kRetryCountProperty).toInt();

    if (statusCode == 401 && withToken && retryCount == 0) {
        const QString path = reply->property(kPathProperty).toString();
        const QByteArray method = reply->property(kMethodProperty).toByteArray();
        const QByteArray body = reply->property(kBodyProperty).toByteArray();

        refreshAndRetry(
            path, method, body, retryCount, std::move(onSuccess),
            std::move(onError)
        );
        return;
    }

    if (statusCode == 401 && withToken && retryCount > 0) {
        clearSession();
    }

    if (reply->error() != QNetworkReply::NoError) {
        onError(handleReplyError(reply));
        return;
    }

    QByteArray rawData = reply->readAll();

    if ((statusCode >= 300 || statusCode < 200) &&
        (rawData.isEmpty() || rawData == "null")) {
        onError("Сервер вернул ошибку: " + QString::number(statusCode));
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);
    if (parseError.error != QJsonParseError::NoError &&
        (statusCode >= 300 || statusCode < 200)) {
        onError("Ошибка разбора ответа сервера");
        return;
    }

    QJsonObject json = doc.object();
    onSuccess(json);
}
