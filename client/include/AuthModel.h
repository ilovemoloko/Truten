#ifndef AUTHMODEL_H
#define AUTHMODEL_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QRegularExpression>
#include "BaseAPIModel.h"

class AuthModel : public BaseModel {
    Q_OBJECT

public:
    explicit AuthModel(
        QNetworkAccessManager *manager,
        QObject *parent = nullptr
    );

    void loginApi(const QString &email, const QString &password);
    void createAccountApi(
        const QString &username,
        const QString &email,
        const QString &password
    );

    static QString hashPassword(const QString &password) {
        return QString(QCryptographicHash::hash(
                           password.toUtf8(), QCryptographicHash::Sha256
        )
                           .toHex());
    }

    static bool isValidEmail(const QString &email) {
        QRegularExpression re(
            R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)"
        );
        return re.match(email).hasMatch();
    }

signals:
    void loginApiFinished(const QJsonObject &reply_data);
    void loginApiError(const QString &err_message);

    void createAccountApiFinished(const QJsonObject &reply_data);
    void createAccountApiError(const QString &err_message);

private:
    void loginApiReply(QNetworkReply *reply);
    void createAccountApiReply(QNetworkReply *reply);
};

#endif  // AUTHMODEL_H
