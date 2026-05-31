#include "GymModel.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkReply>

GymModel::GymModel(QNetworkAccessManager *manager, QObject *parent)
    : BaseModel(manager, parent) {
}

void GymModel::fetchGyms() {//change WITHOUT_TOKEN -> WITH_TOKEN
    QNetworkReply *reply =
        sendGetRequest("/sections/gymList", Token::WITH_TOKEN);
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
        sendPostRequest("/sections/gymList", json, Token::WITH_TOKEN);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply, [this](const QJsonObject &json) { emit gymCreated(json); },
            [this](const QString &err_message) { emit gymError(err_message); }
        );
    });
}

void GymModel::fetchUserStats(const QString &userId) {
    QNetworkReply *reply =
        sendGetRequest("/user/" + userId + "/stats", Token::WITH_TOKEN);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply, [this](const QJsonObject &json) { emit statsLoaded(json); },
            [this](const QString &err_message) { emit gymError(err_message); }
        );
    });
}

void GymModel::fetchUserGainedHours(const QString &userId) {
    QNetworkReply *reply = sendGetRequest(
        "/user/" + userId + "/gainedHours", Token::WITH_TOKEN
    );
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply, [this](const QJsonObject &json) { emit hoursLoaded(json); },
            [this](const QString &err_message) { emit gymError(err_message); }
        );
    });
};

void GymModel::addGymAdmin(const QString &gymId, const QString &userId) {
    if (!isAdmin()) {
        emit gymError("Недостаточно прав");
        return;
    }
    QJsonObject json;
    json["userId"] = userId;
    QNetworkReply *reply = sendPostRequest(
        "/sections/" + gymId + "/admins", json, Token::WITHOUT_TOKEN
    );
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply,
            [this](const QJsonObject &data) { emit gymAdminAdded(data); },
            [this](const QString &err) { emit gymError(err); }
        );
    });
}

void GymModel::addHours(int hours, const QString &userId) {
    if (!isAdmin()) {
        emit gymError("Недостаточно прав");
        return;
    }
    QJsonObject json;
    json["hours"] = hours;

    QNetworkReply *reply = sendPostRequest(
        "/user/" + userId + "/gainedHours", json, Token::WITH_TOKEN
    );
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply, [this](const QJsonObject &json) { emit hoursAdded(json); },
            [this](const QString &err_message) { emit gymError(err_message); }
        );
    });
};
