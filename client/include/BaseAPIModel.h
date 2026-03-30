#ifndef BASEAPIMODEL_H
#define BASEAPIMODEL_H

#include <QNetworkAccessManager>
#include <QObject>
#include "enumToken.h"

class BaseModel : public QObject {
    Q_OBJECT

public:
    explicit BaseModel(
        QNetworkAccessManager *manager,
        QObject *parent = nullptr
    );
    QString baseUrl();

protected:
    QNetworkAccessManager *m_manager;
    static QString m_accessToken;
    static QString m_refreshToken;
    static QString m_userId;
    static QString m_userName;

    QNetworkReply *sendPostRequest(
        const QString &path,
        const QJsonObject &json,
        Token withToken
    );
};

#endif  // BASEAPIMODEL_H
