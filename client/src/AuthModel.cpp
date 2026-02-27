#include "AuthModel.h"
#include <QJsonObject>
#include <QNetworkReply>

AuthModel::AuthModel(QNetworkAccessManager *manager, QObject *parent)
    : BaseModel(manager, parent) {
}

void AuthModel::loginApi(const QString &email, const QString &password) {
    QJsonObject json;
    json["email"] = email;
    json["password"] = password;

    QNetworkReply *reply =
        sendPostRequest("/auth/login", json, Token::WITHOUT_TOKEN);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        loginApiReply(reply);
    });
}

void AuthModel::loginApiReply(QNetworkReply *reply) {
    auto deleteReply = qScopeGuard([reply] { reply->deleteLater(); });

    if (reply->error() != QNetworkReply::NoError) {
        emit loginApiError("Network error: " + reply->errorString());
        return;
    }

    QByteArray rawData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit loginApiError(
            "Invalid server response (JSON error): " + parseError.errorString()
        );
        return;
    }

    QJsonObject json = QJsonObject(doc.object());

    int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    switch (statusCode) {
        case 403: {
            QString serverMessage = json["error"].toString();
            emit loginApiError("403 error: " + serverMessage);
            return;
        }
        case 203: {
            m_accessToken = json["accessToken"].toString();
            m_refreshToken = json["refreshToken"].toString();
            m_userId = json["userId"].toString();
            emit loginApiFinished(json);
            break;
        }
        default:
            emit loginApiError("Unknown status code");
            break;
    }
}

void AuthModel::createAccountApi(
    const QString &email,
    const QString &password
) {
    QJsonObject json;
    json["email"] = email;
    json["password"] = password;

    QNetworkReply *reply =
        sendPostRequest("/auth/createAccount", json, Token::WITHOUT_TOKEN);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        createAccountApiReply(reply);
    });
}

void AuthModel::createAccountApiReply(QNetworkReply *reply) {
    auto deleteReply = qScopeGuard([reply] { reply->deleteLater(); });

    if (reply->error() != QNetworkReply::NoError) {
        emit createAccountApiError("Network error: " + reply->errorString());
        return;
    }

    QByteArray rawData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit createAccountApiError(
            "Invalid server response (JSON error): " + parseError.errorString()
        );
        return;
    }

    QJsonObject json = QJsonObject(doc.object());

    int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    switch (statusCode) {
        case 200: {
            m_accessToken = json["accessToken"].toString();
            m_refreshToken = json["refreshToken"].toString();
            m_userId = json["userId"].toString();
            emit createAccountApiFinished(json);
            break;
        }
        default:
            emit createAccountApiError(
                "Unknown status code: " +
                reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                    .toString()
            );
            break;
    }
}
