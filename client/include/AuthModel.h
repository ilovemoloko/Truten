#ifndef AUTHMODEL_H
#define AUTHMODEL_H

#include <QNetworkAccessManager>
#include <QObject>
#include "BaseAPIModel.h"

class AuthModel : public BaseModel {
    Q_OBJECT

public:
    explicit AuthModel(
        QNetworkAccessManager *manager,
        QObject *parent = nullptr
    );

    void loginApi(const QString &email, const QString &password);
    void createAccountApi(const QString &email, const QString &password);

signals:
    void loginApiFinished(const QJsonObject &reply_data);
    void loginApiError(const QString &err_message);

    void createAccountApiFinished(const QJsonObject &reply_data);
    void createAccountApiError(const QString &err_message);

private slots:

    void loginApiReply(QNetworkReply *reply);
    void createAccountApiReply(QNetworkReply *reply);
};

#endif  // AUTHMODEL_H
