#include "ServerTest.h"
#include <QThread>
#include <QTimer>
#include <QTemporaryDir>
#include <QDir>

namespace {
QString testDbHost() { return qEnvironmentVariable("TRUTEN_TEST_DB_HOST", "localhost"); }
QString testDbPort() { return qEnvironmentVariable("TRUTEN_TEST_DB_PORT", "5432"); }
QString testDbName() { return qEnvironmentVariable("TRUTEN_TEST_DB_NAME", "truten_test"); }
QString testDbUser() { return qEnvironmentVariable("TRUTEN_TEST_DB_USER", "lavrinajtisaleksei"); }
QString testDbPass() { return qEnvironmentVariable("TRUTEN_TEST_DB_PASS", ""); }

void configurePsqlEnvironment(QProcess &psql) {
    auto env = QProcessEnvironment::systemEnvironment();
    const auto pass = testDbPass();
    if (!pass.isEmpty()) {
        env.insert("PGPASSWORD", pass);
    }
    psql.setProcessEnvironment(env);
}

QStringList psqlArgs(const QString &sql) {
    return {
        "-h", testDbHost(),
        "-p", testDbPort(),
        "-U", testDbUser(),
        "-d", testDbName(),
        "-v", "ON_ERROR_STOP=1",
        "-c", sql
    };
}
}

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

Response ServerTest::patch(const QString &path, const QJsonObject &body, const QString &token) {
    return request("PATCH", path, body, token);
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
    configurePsqlEnvironment(psql);
    psql.start("psql", psqlArgs(
        QString("UPDATE users SET is_admin = TRUE WHERE email = '%1';").arg(email)
    ));
    QVERIFY2(psql.waitForFinished(3000), "psql grantAdmin timed out");
    QCOMPARE(psql.exitCode(), 0);
}

void ServerTest::initTestCase() {
    m_serverProcess = new QProcess(this);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("DB_NAME", testDbName());
    env.insert("DB_USER", testDbUser());
    env.insert("DB_PASS", testDbPass());
    env.insert("DB_HOST", testDbHost());
    env.insert("DB_PORT", testDbPort());
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
    configurePsqlEnvironment(psql);
    psql.start("psql", psqlArgs("DROP TABLE IF EXISTS users, slots, gyms;"));
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

void ServerTest::testServerFailsWithoutJwtSecret() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString workingDir = tempDir.path() + "/work";
    QVERIFY(QDir().mkpath(workingDir));

    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove("JWT_SECRET");
    process.setProcessEnvironment(env);
    process.setWorkingDirectory(workingDir);

    const QString serverBin = QString("%1/../../server/build/Server")
                                  .arg(QCoreApplication::applicationDirPath());
    process.start(serverBin, {});
    QVERIFY2(process.waitForFinished(3000), "Server did not fail fast without JWT_SECRET");
    QVERIFY(process.exitCode() != 0);
    QVERIFY(process.readAllStandardError().contains("JWT_SECRET"));
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

void ServerTest::testUserCannotActAsAnotherUser() {
    auto bookAsAdmin = post(
        "/v1/slots/" + m_slotId + "/entries",
        {{"userId", m_adminId}},
        m_userToken
    );
    QCOMPARE(bookAsAdmin.status, 403);

    auto cancelAsAdmin = del(
        "/v1/slots/" + m_slotId + "/entries",
        {{"userId", m_adminId}},
        m_userToken
    );
    QCOMPARE(cancelAsAdmin.status, 403);

    auto queueAsAdmin = post(
        "/v1/queue/" + m_slotId + "/join",
        {{"userId", m_adminId}},
        m_userToken
    );
    QCOMPARE(queueAsAdmin.status, 403);

    auto readAdminData = get(
        "/v1/user/" + m_adminId + "/enrollments",
        m_userToken
    );
    QCOMPARE(readAdminData.status, 403);

    auto modifyAdmin = post(
        "/v1/user/" + m_adminId + "/gainedHours",
        {{"hours", 1}},
        m_userToken
    );
    QCOMPARE(modifyAdmin.status, 403);
}

void ServerTest::testUserCannotManageSlots() {
    auto createR = post("/v1/slots", {
        {"gymId",     m_gymId},
        {"startTime", "2030-02-01T10:00:00"},
        {"endTime",   "2030-02-01T11:00:00"},
        {"capacity",  2}
    }, m_userToken);
    QCOMPARE(createR.status, 403);

    auto patchR = patch("/v1/slots/" + m_slotId, {
        {"startTime", "2030-01-01T10:30:00"},
        {"endTime",   "2030-01-01T11:30:00"},
        {"capacity",  2}
    }, m_userToken);
    QCOMPARE(patchR.status, 403);

    auto deleteR = del("/v1/slots/" + m_slotId, {}, m_userToken);
    QCOMPARE(deleteR.status, 403);
}

void ServerTest::testUserCannotUseAdminApi() {
    auto createGymR = post("/v1/sections/gymList", {
        {"gymName",   "Forbidden Gym"},
        {"creatorId", m_userId}
    }, m_userToken);
    QCOMPARE(createGymR.status, 403);

    auto addAdminR = post(
        "/v1/sections/" + m_gymId + "/admins",
        {{"userId", m_userId}},
        m_userToken
    );
    QCOMPARE(addAdminR.status, 403);

    auto addHoursR = post(
        "/v1/user/" + m_adminId + "/gainedHours",
        {{"hours", 1}},
        m_userToken
    );
    QCOMPARE(addHoursR.status, 403);

    auto banR = post(
        "/v1/user/" + m_adminId + "/isBanned",
        {{"banDuration", 1}},
        m_userToken
    );
    QCOMPARE(banR.status, 403);

    auto deleteAccountR = del(
        "/v1/user/" + m_adminId + "/account",
        {},
        m_userToken
    );
    QCOMPARE(deleteAccountR.status, 403);
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
