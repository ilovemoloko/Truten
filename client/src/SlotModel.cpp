#include "SlotModel.h"
#include "QJsonObject"
#include "QNetworkReply"

SlotModel::SlotModel(QNetworkAccessManager *manager, QObject *parent)
    : BaseModel(manager, parent){};

void SlotModel::fetchSlots(const QString &gymId) {
    QNetworkReply *reply =
        sendGetRequest("/sections/" + gymId + "/slots", Token::WITHOUT_TOKEN);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply, [this](const QJsonObject &data) { emit slotsLoaded(data); },
            [this](const QString &err_message) { emit slotError(err_message); }
        );
    });
};

void SlotModel::bookSlot(const QString &slotId) {
    QJsonObject json;
    json["userId"] = getUserId();

    QNetworkReply *reply = sendPostRequest(
        "/slots/" + slotId + "/entries", json, Token::WITHOUT_TOKEN
    );

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply,
            [this](const QJsonObject &data) { emit bookingFinished(data); },
            [this](const QString &err_message) { emit slotError(err_message); }
        );
    });
};

void SlotModel::cancelBooking(const QString &slotId) {
    QJsonObject json;
    json["userId"] = getUserId();

    QNetworkReply *reply = sendDeleteRequest(
        "/slots/" + slotId + "/entries", json, Token::WITHOUT_TOKEN
    );

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply,
            [this](const QJsonObject &data) { emit bookingFinished(data); },
            [this](const QString &err_message) { emit slotError(err_message); }
        );
    });
};
