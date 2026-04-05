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
        emit apiError(handleReplyError(reply));
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

    onSuccess(statusCode, json);
}

void GymModel::fetchGyms() {
    QNetworkReply *reply = sendGetRequest("/sections/GymList", Token::WITHOUT_TOKEN);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply, [this](int, const QJsonObject &json) {
            emit gymsLoaded(json);
        });
    });
}

void GymModel::fetchSlots(const QString& gymId) {
    QJsonObject body;
    body["gym_id"] = gymId;
    QNetworkReply *reply = sendGetRequest("/slots/list", body, Token::WITHOUT_TOKEN);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply, [this](int, const QJsonObject &json) {
            emit slotsLoaded(json);
        });
    });
}

void GymModel::bookSlot(int slotId) {
    QJsonObject body;
    body["slot_id"] = slotId;
    QNetworkReply *reply = sendPostRequest("/slots/book", body, Token::WITHOUT_TOKEN);
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
        sendPostRequest("/slots/cancel", body, Token::WITHOUT_TOKEN);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply, [this](int, const QJsonObject &json) {
            emit bookingFinished(json);
        });
    });
}

void GymModel::fetchUserStats() {
    QNetworkReply *reply =
        sendPostRequest("/user/stats", {}, Token::WITHOUT_TOKEN);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply, [this](int, const QJsonObject &json) {
            emit statsLoaded(json);
        });
    });
}
