#include "GymModelView.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantMap>

GymModelView::GymModelView(
    GymModel *gymModel,
    SlotModel *slotModel,
    QObject *parent
)
    : QObject(parent), m_gymModel(gymModel), m_slotModel(slotModel) {
    connect(
        m_gymModel, &GymModel::gymsLoaded, this, &GymModelView::onGymsLoaded
    );
    connect(
        m_gymModel, &GymModel::statsLoaded, this, &GymModelView::onStatsLoaded
    );
    connect(m_gymModel, &GymModel::gymError, this, &GymModelView::onApiError);

    connect(
        m_slotModel, &SlotModel::slotsLoaded, this, &GymModelView::onSlotsLoaded
    );
    connect(
        m_slotModel, &SlotModel::bookingFinished, this,
        &GymModelView::onBookingFinished
    );

    connect(
        m_slotModel, &SlotModel::slotError, this, &GymModelView::onApiError
    );
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

    // beginRequest();
    // m_gymModel->fetchUserStats(); // stats for mvp

    beginRequest();
    m_gymModel->fetchGyms();
}


// returns gym's slots
void GymModelView::selectGym(const QString &gymId) {
    m_selectedGymId = gymId;
    emit selectedGymIdChanged();
    m_slots.clear();
    emit slotsChanged();
    loadSlots(m_selectedGymId);
}

void GymModelView::loadSlots(const QString &gymId) {
    beginRequest();
    m_slotModel->fetchSlots(gymId);
}



void GymModelView::bookSlot(const QString &slotId) {
    beginRequest();
    m_slotModel->bookSlot(slotId);
}

void GymModelView::cancelBooking(const QString &slotId) {
    beginRequest();
    m_slotModel->cancelBooking(slotId);
}

void GymModelView::onGymsLoaded(const QJsonObject &data) {
    endRequest();

    m_gyms.clear();
    m_gyms = data["sections"].toArray().toVariantList();

    emit gymsChanged();
}

void GymModelView::onSlotsLoaded(const QJsonObject &data) {
    endRequest();

    // because dayOfWeek() returns [1;7] (returns 0 if date is not valid)
    static const QStringList dayNames = {"",   "ПН", "ВТ", "СР",
                                         "ЧТ", "ПТ", "СБ", "ВС"};

    QJsonArray slotsArray = data["slots"].toArray();
    m_slots.clear();

    for (const QJsonValue &val : slotsArray) {
        QJsonObject slotObj = val.toObject();
        QVariantMap slot;

        slot["id"] = slotObj["slotId"].toString();
        slot["capacity"] = slotObj["capacity"].toInt();
        slot["participantsCount"] = slotObj["participantsCount"].toInt();

        QDateTime startDateTime =
            QDateTime::fromString(slotObj["startTime"].toString(), Qt::ISODate);

        if (startDateTime.isValid()) {
            QDate date = startDateTime.date();
            int dow = date.dayOfWeek();

            slot["dayOfWeek"] = dayNames[dow];
            slot["dateDisplay"] = date.toString("d MMM");
            slot["timeDisplay"] =
                startDateTime.time().toString("HH:mm");

            slot["isoDate"] = date.toString("yyyy-MM-dd");
        }

        slot["participants"] = slotObj["participants"].toArray().toVariantList();
        // TODO: add set of slotsID that are booked by client logic

        m_slots.append(slot);
    }
    emit slotsChanged();
}

void GymModelView::onBookingFinished(const QJsonObject &data) {
    endRequest();
    QString msg = data["message"].toString("Готово");
    emit actionSuccess(msg);
    loadSlots(m_selectedGymId);
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
