#include "BaseAPIModel.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

QString BaseModel::m_accessToken = QString();
QString BaseModel::m_refreshToken = QString();
QString BaseModel::m_userId = QString();

QString BaseModel::baseUrl() {
    static const QString url = QStringLiteral("http://127.0.0.1:8080/v1");
    return url;
}

BaseModel::BaseModel(QNetworkAccessManager *manager, QObject *parent)
    : QObject(parent), m_manager(manager) {
}

QNetworkReply *BaseModel::sendGetRequest(const QString &path, Token withToken) {
    QNetworkRequest request(QUrl(baseUrl() + path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (withToken == Token::WITH_TOKEN) {
        request.setRawHeader(
            "Authorization", "Bearer " + m_accessToken.toUtf8()
        );
    }

    QNetworkReply *reply = m_manager->get(request);

    return reply;
};

QNetworkReply *BaseModel::sendPostRequest(
    const QString &path,
    const QJsonObject &json,
    Token withToken
) {
    QNetworkRequest request(QUrl(baseUrl() + path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (withToken == Token::WITH_TOKEN) {
        request.setRawHeader(
            "Authorization", "Bearer " + m_accessToken.toUtf8()
        );
    }
    QNetworkReply *reply =
        m_manager->post(request, QJsonDocument(json).toJson());

    return reply;
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
};

void BaseModel::handleReply(
    QNetworkReply *reply,
    std::function<void(const QJsonObject &)> onSuccess,
    std::function<void(const QString &)> onError
) {
    auto deleteReply = qScopeGuard([reply] { reply->deleteLater(); });

    if (reply->error() != QNetworkReply::NoError) {
        emit onError(handleReplyError(reply));
        return;
    }

    int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray rawData = reply->readAll();

    if ((statusCode >= 300 || statusCode < 200) && (rawData.isEmpty() || rawData == "null")) {
        emit onError("Сервер вернул ошибку: " + QString::number(statusCode));
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);
    if (parseError.error != QJsonParseError::NoError && (statusCode >= 300 || statusCode < 200)) {
        emit onError("Ошибка разбора ответа сервера");
        return;
    }

    QJsonObject json = doc.object();

    onSuccess(json);
}

QNetworkReply *BaseModel::sendDeleteRequest(
    const QString &path,
    const QJsonObject &json,
    Token withToken
) {
    QNetworkRequest request(QUrl(baseUrl() + path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (withToken == Token::WITH_TOKEN) {
        request.setRawHeader(
            "Authorization", "Bearer " + m_accessToken.toUtf8()
        );
    }
    QNetworkReply *reply = m_manager->sendCustomRequest(
        request, "DELETE", QJsonDocument(json).toJson()
    );

    return reply;
};
