#include <QGuiApplication>
#include <QNetworkAccessManager>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "AuthModel.h"
#include "AuthModelView.h"
#include "GymModel.h"
#include "GymModelView.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QNetworkAccessManager *networkManager = new QNetworkAccessManager(&app);

    AuthModel *authModel = new AuthModel(networkManager, &app);

    AuthModelView *authViewModel = new AuthModelView(authModel, &app);

    GymModel *gymModel = new GymModel(networkManager, &app);

    GymModelView *gymViewModel = new GymModelView(gymModel, &app);

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("authVM", authViewModel);

    const QUrl url(QStringLiteral("qrc:/qt/qml/client/qml/AuthView.qml"));

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection
    );

    engine.load(url);

    return app.exec();
}
