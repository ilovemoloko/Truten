#ifndef AUTHMODELVIEW_H
#define AUTHMODELVIEW_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include "AuthModel.h"

class AuthModelView : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit AuthModelView(AuthModel *model, QObject *parent = nullptr);

    bool isLoading() const { return m_isLoading; }
    QString errorMessage() const { return m_errorMessage; }

    Q_INVOKABLE void login(const QString &email, const QString &password);
    Q_INVOKABLE void createAccount(const QString &email, const QString &password);

signals:
    void isLoadingChanged();
    void errorMessageChanged();

    void authSuccess();

private slots:
    void onApiFinished(const QJsonObject &reply_data);
    void onApiError(const QString &err_message);

private:
    AuthModel *m_model;
    bool m_isLoading = false;
    QString m_errorMessage;

    void setLoading(bool loading);
    void setError(const QString &error);
};

#endif  // AUTHMODELVIEW_H
