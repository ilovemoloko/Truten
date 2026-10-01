#include <QCoreApplication>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTest>
#include <QTimer>
#include "BaseAPIModel.h"

class FakeHttpServer : public QObject {
    Q_OBJECT
public:
    enum class RetryMode { SuccessAfterRefresh, UnauthorizedAfterRefresh };

    explicit FakeHttpServer(QObject *parent = nullptr) : QObject(parent) {
        connect(&server_, &QTcpServer::newConnection, this, [this]() {
            while (server_.hasPendingConnections()) {
                QTcpSocket *socket = server_.nextPendingConnection();
                socket->setProperty("buffer", QByteArray());
                connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
                    QByteArray buffer = socket->property("buffer").toByteArray();
                    buffer += socket->readAll();
                    socket->setProperty("buffer", buffer);

                    const int headerEnd = buffer.indexOf("\r\n\r\n");
                    if (headerEnd < 0) return;

                    int contentLength = 0;
                    const QList<QByteArray> lines = buffer.left(headerEnd).split('\n');
                    for (QByteArray line : lines) {
                        line = line.trimmed();
                        if (line.toLower().startsWith("content-length:")) {
                            contentLength = line.mid(sizeof("content-length:") - 1)
                                                .trimmed()
                                                .toInt();
                        }
                    }
                    if (buffer.size() < headerEnd + 4 + contentLength) return;

                    handleRequest(socket, buffer);
                });
            }
        });
    }

    bool listen() {
        return server_.listen(QHostAddress::LocalHost, 0);
    }

    quint16 port() const { return server_.serverPort(); }
    int protectedRequests() const { return protectedRequests_; }
    int refreshRequests() const { return refreshRequests_; }
    bool sawExpiredToken() const { return sawExpiredToken_; }
    bool sawNewToken() const { return sawNewToken_; }
    bool sawRefreshToken() const { return sawRefreshToken_; }
    void setRetryMode(RetryMode mode) { retryMode_ = mode; }

private:
    void sendJson(QTcpSocket *socket, int status, const QByteArray &json) {
        const QByteArray reason = status == 200 ? "OK" : "Unauthorized";
        QByteArray response = "HTTP/1.1 " + QByteArray::number(status) + " " + reason + "\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Content-Length: " + QByteArray::number(json.size()) + "\r\n";
        response += "Connection: close\r\n\r\n";
        response += json;
        socket->write(response);
        socket->disconnectFromHost();
    }

    void handleRequest(QTcpSocket *socket, const QByteArray &request) {
        const int lineEnd = request.indexOf("\r\n");
        const QByteArray requestLine = request.left(lineEnd);

        if (requestLine.startsWith("POST /v1/auth/refresh ")) {
            ++refreshRequests_;
            sawRefreshToken_ = request.contains("refresh-test-token");
            sendJson(socket, 200, R"({"accessToken":"new-access-token"})");
            return;
        }

        if (requestLine.startsWith("GET /v1/protected ")) {
            ++protectedRequests_;
            const bool oldToken = request.contains("Authorization: Bearer expired-access-token");
            const bool newToken = request.contains("Authorization: Bearer new-access-token");
            sawExpiredToken_ = sawExpiredToken_ || oldToken;
            sawNewToken_ = sawNewToken_ || newToken;

            if (oldToken) {
                sendJson(socket, 401, R"({"error":"expired"})");
                return;
            }

            if (newToken && retryMode_ == RetryMode::SuccessAfterRefresh) {
                sendJson(socket, 200, R"({"ok":true})");
                return;
            }

            sendJson(socket, 401, R"({"error":"still unauthorized"})");
            return;
        }

        sendJson(socket, 401, R"({"error":"unexpected request"})");
    }

    QTcpServer server_;
    RetryMode retryMode_ = RetryMode::SuccessAfterRefresh;
    int protectedRequests_ = 0;
    int refreshRequests_ = 0;
    bool sawExpiredToken_ = false;
    bool sawNewToken_ = false;
    bool sawRefreshToken_ = false;
};

class ClientRefreshTest : public QObject {
    Q_OBJECT
private slots:
    void init() {
        BaseModel::clearSession();
        qunsetenv("TRUTEN_API_BASE_URL");
    }

    void cleanup() {
        BaseModel::clearSession();
        qunsetenv("TRUTEN_API_BASE_URL");
    }

    void refreshesOnceAndRetriesOriginalRequest() {
        FakeHttpServer server;
        QVERIFY(server.listen());
        qputenv(
            "TRUTEN_API_BASE_URL",
            QString("http://127.0.0.1:%1/v1").arg(server.port()).toUtf8()
        );

        QNetworkAccessManager manager;
        BaseModel model(&manager);
        BaseModel::setAccessToken("expired-access-token");
        BaseModel::setRefreshToken("refresh-test-token");

        bool success = false;
        QString error;
        QEventLoop loop;
        QTimer timeout;
        timeout.setSingleShot(true);
        timeout.setInterval(5000);
        connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

        QNetworkReply *reply =
            model.sendGetRequest("/protected", Token::WITH_TOKEN);
        connect(reply, &QNetworkReply::finished, &model, [&]() {
            model.handleReply(
                reply,
                [&](const QJsonObject &json) {
                    success = json["ok"].toBool();
                    loop.quit();
                },
                [&](const QString &message) {
                    error = message;
                    loop.quit();
                }
            );
        });

        timeout.start();
        loop.exec();

        QVERIFY2(timeout.isActive(), "refresh test timed out");
        QVERIFY2(success, qPrintable(error));
        QCOMPARE(server.protectedRequests(), 2);
        QCOMPARE(server.refreshRequests(), 1);
        QVERIFY(server.sawExpiredToken());
        QVERIFY(server.sawNewToken());
        QVERIFY(server.sawRefreshToken());
        QCOMPARE(BaseModel::getAccessToken(), QString("new-access-token"));
    }

    void neverRefreshesMoreThanOnce() {
        FakeHttpServer server;
        server.setRetryMode(FakeHttpServer::RetryMode::UnauthorizedAfterRefresh);
        QVERIFY(server.listen());
        qputenv(
            "TRUTEN_API_BASE_URL",
            QString("http://127.0.0.1:%1/v1").arg(server.port()).toUtf8()
        );

        QNetworkAccessManager manager;
        BaseModel model(&manager);
        BaseModel::setAccessToken("expired-access-token");
        BaseModel::setRefreshToken("refresh-test-token");

        bool success = false;
        QString error;
        QEventLoop loop;
        QTimer timeout;
        timeout.setSingleShot(true);
        timeout.setInterval(5000);
        connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

        QNetworkReply *reply =
            model.sendGetRequest("/protected", Token::WITH_TOKEN);
        connect(reply, &QNetworkReply::finished, &model, [&]() {
            model.handleReply(
                reply,
                [&](const QJsonObject &) {
                    success = true;
                    loop.quit();
                },
                [&](const QString &message) {
                    error = message;
                    loop.quit();
                }
            );
        });

        timeout.start();
        loop.exec();

        QVERIFY2(timeout.isActive(), "retry-limit test timed out");
        QVERIFY(!success);
        QVERIFY(!error.isEmpty());
        QCOMPARE(server.protectedRequests(), 2);
        QCOMPARE(server.refreshRequests(), 1);
        QVERIFY(BaseModel::getAccessToken().isEmpty());
        QVERIFY(BaseModel::getRefreshToken().isEmpty());
    }
};

QTEST_MAIN(ClientRefreshTest)
#include "ClientRefreshTest.moc"
