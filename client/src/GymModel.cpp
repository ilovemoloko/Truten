#include "GymModel.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkReply>

GymModel::GymModel(QNetworkAccessManager *manager, QObject *parent)
    : BaseModel(manager, parent) {
}

void GymModel::fetchGyms() {
    QNetworkReply *reply =
        sendGetRequest("/sections/gymList", Token::WITHOUT_TOKEN);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply, [this](const QJsonObject &json) { emit gymsLoaded(json); },
            [this](const QString &err_message) { emit gymError(err_message); }
        );
    });
}

void GymModel::fetchUserStats(const QString &userId) {
    QNetworkReply *reply =
        sendGetRequest("/user/" + userId + "/stats", Token::WITHOUT_TOKEN);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply, [this](const QJsonObject &json) { emit statsLoaded(json); },
            [this](const QString &err_message) { emit gymError(err_message); }
        );
    });
}
