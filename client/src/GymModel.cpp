#include "GymModel.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkReply>
#include <functional>

GymModel::GymModel(QNetworkAccessManager *manager, QObject *parent)
    : BaseModel(manager, parent) {
}

void GymModel::handleReply(
    QNetworkReply *reply,
    std::function<void(int, const QJsonObject &)> onSuccess
) {
    auto deleteReply = qScopeGuard([reply] { reply->deleteLater(); });

    if (reply->error() != QNetworkReply::NoError) {
        emit apiError("Ошибка сети: " + reply->errorString());
        return;
    }

    int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray rawData = reply->readAll();

    if (rawData.isEmpty() || rawData == "null") {
        emit apiError("Сервер вернул ошибку: " + QString::number(statusCode));
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        emit apiError("Ошибка разбора ответа сервера");
        return;
    }

    QJsonObject json = doc.object();
    if (statusCode >= 400) {
        emit apiError(json["error"].toString("Ошибка сервера"));
        return;
    }

    onSuccess(statusCode, json);
}

void GymModel::fetchGyms() {
    QNetworkReply *reply = sendPostRequest("/gyms/list", {}, Token::WITH_TOKEN);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply, [this](int, const QJsonObject &json) {
            emit gymsLoaded(json);
        });
    });
}

void GymModel::fetchSlots(int gymId) {
    QJsonObject body;
    body["gym_id"] = gymId;
    QNetworkReply *reply = sendPostRequest("/slots/list", body, Token::WITH_TOKEN);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply, [this](int, const QJsonObject &json) {
            emit slotsLoaded(json);
        });
    });
}

void GymModel::bookSlot(int slotId) {
    QJsonObject body;
    body["slot_id"] = slotId;
    QNetworkReply *reply = sendPostRequest("/slots/book", body, Token::WITH_TOKEN);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply, [this](int, const QJsonObject &json) {
            emit bookingFinished(json);
        });
    });
}

void GymModel::cancelBooking(int slotId) {
    QJsonObject body;
    body["slot_id"] = slotId;
    QNetworkReply *reply =
        sendPostRequest("/slots/cancel", body, Token::WITH_TOKEN);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply, [this](int, const QJsonObject &json) {
            emit bookingFinished(json);
        });
    });
}

void GymModel::fetchUserStats() {
    QNetworkReply *reply =
        sendPostRequest("/user/stats", {}, Token::WITH_TOKEN);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply, [this](int, const QJsonObject &json) {
            emit statsLoaded(json);
        });
    });
}
