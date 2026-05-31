#include "ServerTest.h"
#include <QThread>
#include <QTimer>

ServerTest::ServerTest(QObject *parent) : QObject(parent) {}

Response ServerTest::request(
    const QByteArray &method,
    const QString &path,
    const QJsonObject &body,
    const QString &token
) {
    QNetworkRequest req(QUrl(QString(SERVER_URL) + path));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!token.isEmpty()) {
        req.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    }

    const QByteArray data = body.isEmpty() ? QByteArray() : QJsonDocument(body).toJson();

    QNetworkReply *reply = nullptr;
    if (method == "GET") {
        reply = m_net.get(req);
    } else if (method == "POST") {
        reply = m_net.post(req, data);
    } else {
        reply = m_net.sendCustomRequest(req, method, data);
    }

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
    reply->deleteLater();
    return {status, json};
}

Response ServerTest::get(const QString &path, const QString &token) {
    return request("GET", path, {}, token);
}

Response ServerTest::post(const QString &path, const QJsonObject &body, const QString &token) {
    return request("POST", path, body, token);
}

Response ServerTest::del(const QString &path, const QJsonObject &body, const QString &token) {
    return request("DELETE", path, body, token);
}

bool ServerTest::waitForServer(int timeoutMs) {
    const auto deadline = QDateTime::currentMSecsSinceEpoch() + timeoutMs;
    while (QDateTime::currentMSecsSinceEpoch() < deadline) {
        auto r = get("/");
        if (r.status != 0) return true;
        QThread::msleep(200);
    }
    return false;
}

void ServerTest::grantAdmin(const QString &email) {
    QProcess psql;
    psql.start("psql", {
        "-U", "lavrinajtisaleksei",
        "-d", "truten_test",
        "-c", QString("UPDATE users SET is_admin = TRUE WHERE email = '%1';").arg(email)
    });
    psql.waitForFinished(3000);
}

void ServerTest::initTestCase() {
    m_serverProcess = new QProcess(this);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("DB_NAME", "truten_test");
    env.insert("DB_USER", "lavrinajtisaleksei");
    env.insert("DB_PASS", "");
    env.insert("DB_HOST", "localhost");
    env.insert("DB_PORT", "5432");
    env.insert("JWT_SECRET", "test-secret");
    env.insert("PORT", "8081");
    m_serverProcess->setProcessEnvironment(env);

    const QString serverBin = QString("%1/../../server/build/Server")
                                  .arg(QCoreApplication::applicationDirPath());
    m_serverProcess->start(serverBin, {});

    QVERIFY2(waitForServer(), "Server did not start within timeout");
}

void ServerTest::cleanupTestCase() {
    if (m_serverProcess) {
        m_serverProcess->terminate();
        m_serverProcess->waitForFinished(3000);
    }
    QProcess psql;
    psql.start("psql", {"-U", "lavrinajtisaleksei", "-d", "truten_test",
                        "-c", "DROP TABLE IF EXISTS users, slots, gyms;"});
    psql.waitForFinished(3000);
}

void ServerTest::testRegisterAdmin() {
    auto r = post("/v1/auth/createAccount", {
        {"email",    "admin@test.com"},
        {"password", "adminpass"},
        {"name",     "Admin"}
    });
    QCOMPARE(r.status, 200);
    QVERIFY(r.json.contains("accessToken"));
    m_adminId = r.json["userId"].toString();
}

void ServerTest::testRegisterUser() {
    auto r = post("/v1/auth/createAccount", {
        {"email",    "user@test.com"},
        {"password", "userpass"},
        {"name",     "User"}
    });
    QCOMPARE(r.status, 200);
    m_userId = r.json["userId"].toString();
}

void ServerTest::testLoginAdmin() {
    grantAdmin("admin@test.com");

    auto r = post("/v1/auth/login", {
        {"email",    "admin@test.com"},
        {"password", "adminpass"}
    });
    QCOMPARE(r.status, 200);
    QVERIFY(r.json.contains("accessToken"));
    m_adminToken = r.json["accessToken"].toString();
    QVERIFY(!m_adminToken.isEmpty());
}

void ServerTest::testLoginUser() {
    auto r = post("/v1/auth/login", {
        {"email",    "user@test.com"},
        {"password", "userpass"}
    });
    QCOMPARE(r.status, 200);
    m_userToken = r.json["accessToken"].toString();
    QVERIFY(!m_userToken.isEmpty());
}

void ServerTest::testCreateGym() {
    auto r = post("/v1/sections/gymList", {
        {"gymName",   "Test Gym"},
        {"creatorId", m_adminId}
    }, m_adminToken);
    QCOMPARE(r.status, 200);
}

void ServerTest::testGetGyms() {
    auto r = get("/v1/sections/gymList", m_adminToken);
    QCOMPARE(r.status, 200);
    QVERIFY(r.json.contains("sections"));

    const auto sections = r.json["sections"].toArray();
    for (const auto &s : sections) {
        if (s.toObject()["name"].toString() == "Test Gym") {
            m_gymId = s.toObject()["id"].toString();
        }
    }
    QVERIFY(!m_gymId.isEmpty());
}

void ServerTest::testCreateSlot() {
    auto r = post("/v1/slots", {
        {"gymId",     m_gymId},
        {"startTime", "2030-01-01T10:00:00"},
        {"endTime",   "2030-01-01T11:00:00"},
        {"capacity",  1}
    }, m_adminToken);
    QCOMPARE(r.status, 200);
}

void ServerTest::testGetSlots() {
    auto r = get("/v1/sections/" + m_gymId + "/slots", m_adminToken);
    QCOMPARE(r.status, 200);
    QVERIFY(r.json.contains("slots"));

    const auto slotArr = r.json["slots"].toArray();
    QVERIFY(!slotArr.isEmpty());
    m_slotId = slotArr.at(0).toObject()["slotId"].toString();
    QVERIFY(!m_slotId.isEmpty());
}

void ServerTest::testBookSlot() {
    auto r = post("/v1/slots/" + m_slotId + "/entries",
                  {{"userId", m_adminId}}, m_adminToken);
    QCOMPARE(r.status, 200);
}

void ServerTest::testBookSlotAlreadyEnrolled() {
    auto r = post("/v1/slots/" + m_slotId + "/entries",
                  {{"userId", m_adminId}}, m_adminToken);
    QCOMPARE(r.status, 400);
    QVERIFY(r.json["error"].toString().contains("Already enrolled"));
}

void ServerTest::testBookSlotFull() {
    auto r = post("/v1/slots/" + m_slotId + "/entries",
                  {{"userId", m_userId}}, m_userToken);
    QCOMPARE(r.status, 400);
    QVERIFY(r.json["error"].toString().contains("full"));
}

void ServerTest::testJoinQueue() {
    auto r = post("/v1/queue/" + m_slotId + "/join",
                  {{"userId", m_userId}}, m_userToken);
    QCOMPARE(r.status, 200);
    QCOMPARE(r.json["enrolled"].toBool(), false);
    QCOMPARE(r.json["queued"].toBool(), true);
}

void ServerTest::testJoinQueueAlreadyIn() {
    auto r = post("/v1/queue/" + m_slotId + "/join",
                  {{"userId", m_userId}}, m_userToken);
    QCOMPARE(r.status, 400);
    QVERIFY(r.json["error"].toString().contains("queue"));
}

void ServerTest::testCancelAndAutoPromote() {
    auto r = del("/v1/slots/" + m_slotId + "/entries",
                 {{"userId", m_adminId}}, m_adminToken);
    QCOMPARE(r.status, 204);

    auto enrolled = get("/v1/user/" + m_userId + "/enrollments", m_userToken);
    QCOMPARE(enrolled.status, 200);
    const auto list = enrolled.json["enrollments"].toArray();
    bool found = false;
    for (const auto &e : list) {
        if (e.toString() == m_slotId) found = true;
    }
    QVERIFY2(found, "User was not auto-promoted from queue to enrolled");

    auto queued = get("/v1/user/" + m_userId + "/queuedSlots", m_userToken);
    QCOMPARE(queued.status, 200);
    for (const auto &q : queued.json["queuedSlots"].toArray()) {
        QVERIFY2(q.toString() != m_slotId, "User still appears in queue after promotion");
    }
}

void ServerTest::testLeaveQueue() {
    auto createR = post("/v1/slots", {
        {"gymId",     m_gymId},
        {"startTime", "2030-01-02T10:00:00"},
        {"endTime",   "2030-01-02T11:00:00"},
        {"capacity",  1}
    }, m_adminToken);
    QCOMPARE(createR.status, 200);

    auto slotsR = get("/v1/sections/" + m_gymId + "/slots", m_adminToken);
    QString newSlotId;
    for (const auto &s : slotsR.json["slots"].toArray()) {
        const QString id = s.toObject()["slotId"].toString();
        if (id != m_slotId) newSlotId = id;
    }
    QVERIFY(!newSlotId.isEmpty());

    post("/v1/slots/" + newSlotId + "/entries", {{"userId", m_adminId}}, m_adminToken);

    auto joinR = post("/v1/queue/" + newSlotId + "/join",
                      {{"userId", m_userId}}, m_userToken);
    QCOMPARE(joinR.json["queued"].toBool(), true);

    auto leaveR = post("/v1/queue/" + newSlotId + "/leave",
                       {{"userId", m_userId}}, m_userToken);
    QCOMPARE(leaveR.status, 204);

    auto queued = get("/v1/user/" + m_userId + "/queuedSlots", m_userToken);
    for (const auto &q : queued.json["queuedSlots"].toArray()) {
        QVERIFY(q.toString() != newSlotId);
    }
}

QTEST_MAIN(ServerTest)
