#ifndef BASEAPIMODEL_H
#define BASEAPIMODEL_H

#include <QJsonObject>
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

    static QString getUserId() {
        return m_userId;
    };

    static void setUserId(const QString &userId) {
        m_userId = userId;
    };

    QNetworkReply *sendPostRequest(
        const QString &path,
        const QJsonObject &json,
        Token withToken
    );

    QNetworkReply *sendDeleteRequest(
        const QString &path,
        const QJsonObject &json,
        Token withToken
    );

    QNetworkReply *sendGetRequest(const QString &path, Token withToken);

    QString handleReplyError(QNetworkReply *reply);

    void handleReply(
        QNetworkReply *reply,
        std::function<void(const QJsonObject &)> onSuccess,
        std::function<void(const QString &)> onError
    );

private:
    static QString m_accessToken;
    static QString m_refreshToken;
    static QString m_userId;
};

#endif  // BASEAPIMODEL_H
