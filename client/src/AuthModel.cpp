#include "AuthModel.h"
#include <QJsonObject>
#include <QNetworkReply>

AuthModel::AuthModel(QNetworkAccessManager *manager, QObject *parent)
    : BaseModel(manager, parent) {
}

void AuthModel::loginApi(const QString &email, const QString &password) {

    if(!isValidEmail(email)){
        emit loginApiError("С твоей почтой что-то не так...");
        return;
    }

    QJsonObject json;
    json["email"] = email;
    json["password"] = hashPassword(password);

    QNetworkReply *reply =
        sendPostRequest("/auth/login", json, Token::WITHOUT_TOKEN);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        loginApiReply(reply);
    });
}

void AuthModel::loginApiReply(QNetworkReply *reply) {
    auto deleteReply = qScopeGuard([reply] { reply->deleteLater(); });

    if (reply->error() != QNetworkReply::NoError) {
        emit loginApiError(handleReplyError(reply));
        return;
    }

    int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    QByteArray rawData = reply->readAll();
    qDebug() << "Raw data from server:" << rawData;

    if (rawData.isEmpty() || rawData == "null") {
        qDebug() << "Ошибка сервера, код:" << statusCode;
        emit loginApiError(
            "ServerError: Сервер вернул ошибку " + QString::number(statusCode)
        );
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "Invalid server response (JSON error): " + parseError.errorString() << "\n";

        emit loginApiError(
            "ParseError: Неправильный формат ответа на сервере"
        );
        return;
    }

    QJsonObject json = QJsonObject(doc.object());

    switch (statusCode) {
        case 200: {
            // m_accessToken = json["accessToken"].toString(); add for JWT
            // m_refreshToken = json["refreshToken"].toString();
            setUserId(json["userId"].toString());
            emit loginApiFinished(json);
            break;
        }
        default:
            qDebug() << "Неизвестный статус код: " + QString::number(statusCode) +
                            json["error"].toString() <<  '\n';
            emit loginApiError(
                "StatusCodeError: Неизвестный статус код: " + QString::number(statusCode)
            );
            break;
    }
}

void AuthModel::createAccountApi(
    const QString &name,
    const QString &email,
    const QString &password
) {
    if(!isValidEmail(email)){
        emit createAccountApiError("С твоей почтой что-то не так...");
        return;
    }
    QJsonObject json;
    json["name"] = name;
    json["email"] = email;
    json["password"] = hashPassword(password);

    QNetworkReply *reply =
        sendPostRequest("/auth/createAccount", json, Token::WITHOUT_TOKEN);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        createAccountApiReply(reply);
    });
}

void AuthModel::createAccountApiReply(QNetworkReply *reply) {
    auto deleteReply = qScopeGuard([reply] { reply->deleteLater(); });

    if (reply->error() != QNetworkReply::NoError) {
        emit createAccountApiError(handleReplyError(reply));
        return;
    }

    int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    QByteArray rawData = reply->readAll();

    qDebug() << "Raw data from server:" << rawData;

    if (rawData.isEmpty() || rawData == "null") {
        qDebug() << "Ошибка сервера, код:" << statusCode;
        emit loginApiError(
            "ServerError: Сервер вернул ошибку: " + QString::number(statusCode)
        );
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "Invalid server response (JSON error): " + parseError.errorString() << "\n";
        emit createAccountApiError(
            "ParseError: Неправильный формат ответа на сервере"
        );
        return;
    }

    QJsonObject json = QJsonObject(doc.object());

    switch (statusCode) {
        case 200: {
            // m_accessToken = json["accessToken"].toString(); add for JWT
            // m_refreshToken = json["refreshToken"].toString();
            setUserId(json["userId"].toString());
            emit createAccountApiFinished(json);
            break;
        }
        default:
            QString serverMessage = json["error"].toString();
            qDebug() <<                 "Unknown status code: " + QString::number(statusCode) +
                            serverMessage << '\n';
            emit createAccountApiError(  // if unknow statusCode returns json
                "StatusCodeError: Произошла ошибка на сервере, попробуйте еще"
            );
            break;
    }
}
