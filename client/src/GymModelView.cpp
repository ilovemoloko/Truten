#include "GymModelView.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantMap>

GymModelView::GymModelView(GymModel *model, QObject *parent)
    : QObject(parent), m_model(model) {
    connect(m_model, &GymModel::gymsLoaded, this, &GymModelView::onGymsLoaded);
    connect(
        m_model, &GymModel::slotsLoaded, this, &GymModelView::onSlotsLoaded
    );
    connect(
        m_model,
        &GymModel::bookingFinished,
        this,
        &GymModelView::onBookingFinished
    );
    connect(m_model, &GymModel::statsLoaded, this, &GymModelView::onStatsLoaded);
    connect(m_model, &GymModel::apiError, this, &GymModelView::onApiError);
}

void GymModelView::beginRequest() {
    m_pendingRequests++;
    if (m_pendingRequests == 1) {
        m_isLoading = true;
        emit isLoadingChanged();
    }
}

void GymModelView::endRequest() {
    m_pendingRequests = qMax(0, m_pendingRequests - 1);
    if (m_pendingRequests == 0) {
        m_isLoading = false;
        emit isLoadingChanged();
    }
}

void GymModelView::setError(const QString &error) {
    m_errorMessage = error;
    emit errorMessageChanged();
    endRequest();
}

void GymModelView::init() {
    m_errorMessage.clear();
    emit errorMessageChanged();

    beginRequest();
    m_model->fetchUserStats();

    beginRequest();
    m_model->fetchGyms();
}

void GymModelView::selectGym(int gymId) {
    if (m_selectedGymId == gymId)
        return;
    m_selectedGymId = gymId;
    emit selectedGymIdChanged();
    m_slots.clear();
    emit slotsChanged();
    reloadSlots();
}

void GymModelView::reloadSlots() {
    if (m_selectedGymId < 0)
        return;
    beginRequest();
    m_model->fetchSlots(m_selectedGymId);
}

void GymModelView::bookSlot(int slotId) {
    beginRequest();
    m_model->bookSlot(slotId);
}

void GymModelView::cancelBooking(int slotId) {
    beginRequest();
    m_model->cancelBooking(slotId);
}

void GymModelView::onGymsLoaded(const QJsonObject &data) {
    endRequest();

    QJsonArray gymsArray = data["gyms"].toArray();
    m_gyms.clear();
    for (const QJsonValue &val : gymsArray) {
        QJsonObject gymObj = val.toObject();
        QVariantMap gym;
        gym["id"] = gymObj["id"].toInt();
        gym["name"] = gymObj["name"].toString();
        m_gyms.append(gym);
    }
    emit gymsChanged();

    // Auto-select first gym
    if (!m_gyms.isEmpty() && m_selectedGymId < 0) {
        int firstId = m_gyms.first().toMap()["id"].toInt();
        m_selectedGymId = firstId;
        emit selectedGymIdChanged();
        reloadSlots();
    }
}

void GymModelView::onSlotsLoaded(const QJsonObject &data) {
    endRequest();

    static const QStringList dayNames = {
        "", "ПН", "ВТ", "СР", "ЧТ", "ПТ", "СБ", "ВС"
    };

    QJsonArray slotsArray = data["slots"].toArray();
    m_slots.clear();
    for (const QJsonValue &val : slotsArray) {
        QJsonObject slotObj = val.toObject();
        QVariantMap slot;
        slot["id"] = slotObj["id"].toInt();
        slot["activity"] = slotObj["activity"].toString();
        slot["time"] = slotObj["time"].toString();
        slot["occupied"] = slotObj["occupied"].toInt();
        slot["capacity"] = slotObj["capacity"].toInt();
        slot["isBooked"] = slotObj["is_booked"].toBool();
        slot["inQueue"] = slotObj["in_queue"].toBool();
        slot["queuePosition"] = slotObj["queue_position"].toInt();

        QString dateStr = slotObj["date"].toString();
        QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
        if (date.isValid()) {
            int dow = date.dayOfWeek(); 
            slot["dayOfWeek"] = dow < dayNames.size() ? dayNames[dow] : "";
            slot["date"] = date.toString("d MMM");
        } else {
            slot["dayOfWeek"] = "";
            slot["date"] = dateStr;
        }

        m_slots.append(slot);
    }
    emit slotsChanged();
}

void GymModelView::onBookingFinished(const QJsonObject &data) {
    endRequest();
    QString msg = data["message"].toString("Готово");
    emit actionSuccess(msg);
    reloadSlots();
}

void GymModelView::onStatsLoaded(const QJsonObject &data) {
    endRequest();
    m_userName = data["name"].toString();
    m_visitCount = data["visit_count"].toInt();
    m_visitsNeeded = data["visits_needed"].toInt(8);
    emit userInfoChanged();
}

void GymModelView::onApiError(const QString &message) {
    setError(message);
}
