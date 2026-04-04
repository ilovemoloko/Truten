#include "BaseAPIModel.h"
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>

QString BaseModel::m_accessToken = QString();
QString BaseModel::m_refreshToken = QString();
QString BaseModel::m_userId = QString();
QString BaseModel::m_userName = QString();

QString BaseModel::baseUrl() {
    static const QString url = QStringLiteral("http://127.0.0.1:8080/v1");
    return url;
}

BaseModel::BaseModel(QNetworkAccessManager *manager, QObject *parent)
    : QObject(parent), m_manager(manager) {
}

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
    reply->setParent(this
    );  // if connect() failed for some reason, parent will delete memory
    return reply;
}
