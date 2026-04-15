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

void GymModel::createGym(const QString &name) {
    if (!isAdmin()) {
        emit gymError("Недостаточно прав");
        return;
    }

    QJsonObject json;
    json["gymName"] = name;
    json["creatorId"] = getUserId();
    QNetworkReply *reply =
        sendPostRequest("/sections/gymList", json, Token::WITHOUT_TOKEN);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply, [this](const QJsonObject &json) { emit gymCreated(json); },
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

void GymModel::fetchUserGainedHours(const QString &userId) {
    QNetworkReply *reply = sendGetRequest(
        "/user/" + userId + "/gainedHours", Token::WITHOUT_TOKEN
    );
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply, [this](const QJsonObject &json) { emit hoursLoaded(json); },
            [this](const QString &err_message) { emit gymError(err_message); }
        );
    });
};

void GymModel::addHours(int hours, const QString &userId) {
    if (!isAdmin()) {
        emit gymError("Недостаточно прав");
        return;
    }
    QJsonObject json;
    json["hours"] = hours;

    QNetworkReply *reply = sendPostRequest(
        "/user/" + userId + "/gainedHours", json, Token::WITHOUT_TOKEN
    );
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply, [this](const QJsonObject &json) { emit hoursAdded(json); },
            [this](const QString &err_message) { emit gymError(err_message); }
        );
    });
};
