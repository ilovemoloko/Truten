#include "AuthModelView.h"
#include <QDebug>

AuthModelView::AuthModelView(AuthModel *model, QObject *parent)
    : QObject(parent), m_model(model)
{
    connect(m_model, &AuthModel::loginApiFinished,
            this, &AuthModelView::onApiFinished);
    connect(m_model, &AuthModel::loginApiError,
            this, &AuthModelView::onApiError);

    connect(m_model, &AuthModel::createAccountApiFinished,
            this, &AuthModelView::onApiFinished);
    connect(m_model, &AuthModel::createAccountApiError,
            this, &AuthModelView::onApiError);
}

void AuthModelView::login(const QString &email, const QString &password) {
    setLoading(true);
    setError("");

    m_model->loginApi(email, password);
}

void AuthModelView::createAccount(const QString &email, const QString &password) {
    setLoading(true);
    setError("");

    m_model->createAccountApi(email, password);
}

void AuthModelView::onApiFinished(const QJsonObject &reply_data) {
    setLoading(false);

    qDebug() << "Успешный ответ от сервера:" << reply_data;

    emit authSuccess();
}

void AuthModelView::onApiError(const QString &err_message) {
    setLoading(false);

    setError(err_message);
    qDebug() << "Ошибка авторизации:" << err_message;
}


void AuthModelView::setLoading(bool loading) {
    if (m_isLoading == loading) return;

    m_isLoading = loading;
    emit isLoadingChanged();
}

void AuthModelView::setError(const QString &error) {
    if (m_errorMessage == error) return;

    m_errorMessage = error;
    emit errorMessageChanged();
}
