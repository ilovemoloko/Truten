#include "SlotModel.h"
#include "QJsonObject"
#include "QNetworkReply"

SlotModel::SlotModel(QNetworkAccessManager *manager, QObject *parent)
    : BaseModel(manager, parent){};

void SlotModel::fetchSlots(const QString &gymId) {//change WITHOUT_TOKEN -> WITH_TOKEN
    QNetworkReply *reply =
        sendGetRequest("/sections/" + gymId + "/slots", Token::WITH_TOKEN);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply, [this](const QJsonObject &data) { emit slotsLoaded(data); },
            [this](const QString &err_message) { emit slotError(err_message); }
        );
    });
};

void SlotModel::createSlot(
    const QDateTime &startTime,
    const QDateTime &endTime,
    const QString &gymId,
    int capacity
) {
    if (!isAdmin()) {
        qDebug() << "Error: Not enough rights";
        emit slotError("Недостаточно прав");
        return;
    }

    QJsonObject json;
    json["startTime"] = startTime.toString(Qt::ISODate);
    json["endTime"] = endTime.toString(Qt::ISODate);
    json["gymId"] = gymId;
    json["capacity"] = capacity;

    QNetworkReply *reply =
        sendPostRequest("/slots", json, Token::WITH_TOKEN);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply, [this](const QJsonObject &data) { emit slotCreated(data); },
            [this](const QString &err_message) { emit slotError(err_message); }
        );
    });
};

void SlotModel::removeSlot(const QString &slotId) {
    if (!isAdmin()) {
        qDebug() << "Error: Not enough rights";
        emit slotError("Недостаточно прав");
        return;
    }
    QNetworkReply *reply =
        sendDeleteRequest("/slots/" + slotId, {}, Token::WITH_TOKEN);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply, [this](const QJsonObject &data) { emit slotRemoved(data); },
            [this](const QString &err_message) { emit slotError(err_message); }
        );
    });
}

void SlotModel::bookSlot(const QString &slotId) {
    QJsonObject json;
    json["userId"] = getUserId();

    QNetworkReply *reply = sendPostRequest(
        "/slots/" + slotId + "/entries", json, Token::WITH_TOKEN
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
        "/slots/" + slotId + "/entries", json, Token::WITH_TOKEN
    );

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply,
            [this](const QJsonObject &data) { emit bookingFinished(data); },
            [this](const QString &err_message) { emit slotError(err_message); }
        );
    });
};

void SlotModel::getBookedSlotsIds(const QString &userId) {
    QNetworkReply *reply = sendGetRequest(
        "/user/" + userId + "/enrollments", Token::WITH_TOKEN
    );

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply,
            [this](const QJsonObject &data) {
                emit bookedSlotsIdsFinished(data);
            },
            [this](const QString &err_message) { emit slotError(err_message); }
        );
    });
};
//add queue methods
void SlotModel::joinQueue(const QString &slotId) {
    QJsonObject json;
    json["userId"] = getUserId();

    QNetworkReply *reply = sendPostRequest(
        "/queue/" + slotId + "/join", json, Token::WITH_TOKEN
    );
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply,
            [this](const QJsonObject &data) { emit queueJoinFinished(data); },
            [this](const QString &err_message) { emit slotError(err_message); }
        );
    });
}

void SlotModel::leaveQueue(const QString &slotId) {
    QJsonObject json;
    json["userId"] = getUserId();

    QNetworkReply *reply = sendPostRequest(
        "/queue/" + slotId + "/leave", json, Token::WITH_TOKEN
    );
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply,
            [this](const QJsonObject &data) { emit queueLeaveFinished(data); },
            [this](const QString &err_message) { emit slotError(err_message); }
        );
    });
}

void SlotModel::getQueuedSlotsIds(const QString &userId) {
    QNetworkReply *reply = sendGetRequest(
        "/user/" + userId + "/queuedSlots", Token::WITH_TOKEN
    );
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(
            reply,
            [this](const QJsonObject &data) { emit queuedSlotsIdsFinished(data); },
            [this](const QString &err_message) { emit slotError(err_message); }
        );
    });
}
