#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QProcess>
#include <QtTest>

static constexpr const char *SERVER_URL = "http://localhost:8081";

struct Response {
    int status;
    QJsonObject json;
};

class ServerTest : public QObject {
    Q_OBJECT

public:
    explicit ServerTest(QObject *parent = nullptr);

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testRegisterAdmin();
    void testRegisterUser();
    void testLoginAdmin();
    void testLoginUser();

    void testCreateGym();
    void testGetGyms();

    void testCreateSlot();
    void testGetSlots();

    void testBookSlot();
    void testBookSlotAlreadyEnrolled();
    void testBookSlotFull();

    void testJoinQueue();
    void testJoinQueueAlreadyIn();
    void testCancelAndAutoPromote();
    void testLeaveQueue();

private:
    Response request(
        const QByteArray &method,
        const QString &path,
        const QJsonObject &body = {},
        const QString &token = {}
    );

    Response get(const QString &path, const QString &token = {});
    Response post(const QString &path, const QJsonObject &body, const QString &token = {});
    Response del(const QString &path, const QJsonObject &body, const QString &token = {});

    bool waitForServer(int timeoutMs = 8000);
    void grantAdmin(const QString &email);

    QProcess *m_serverProcess{nullptr};
    QNetworkAccessManager m_net;

    QString m_adminToken;
    QString m_userToken;
    QString m_adminId;
    QString m_userId;
    QString m_gymId;
    QString m_slotId;
};
