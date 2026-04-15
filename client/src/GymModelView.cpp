#include "GymModelView.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QSortFilterProxyModel>
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
        m_slotModel, &SlotModel::slotCreated, this, &GymModelView::onSlotCreated
    );
    connect(
        m_slotModel, &SlotModel::slotRemoved, this, &GymModelView::onSlotRemoved
    );
    connect(
        m_gymModel, &GymModel::gymCreated, this, &GymModelView::onGymCreated
    );
    connect(
        m_gymModel, &GymModel::hoursLoaded, this, &GymModelView::onHoursLoaded
    );
    connect(
        m_gymModel, &GymModel::hoursAdded, this, &GymModelView::onHoursAdded
    );

    connect(
        m_slotModel, &SlotModel::slotError, this, &GymModelView::onApiError
    );

    connect(
        m_slotModel, &SlotModel::bookedSlotsIdsFinished, this,
        &GymModelView::onBookedSlotsIdsFinished
    );

    m_slotsListModel = new SlotsListModel(this);

    for (int i = 1; i <= 7; i++) {
        auto *proxy = new QSortFilterProxyModel(this);
        proxy->setSourceModel(m_slotsListModel);
        proxy->setFilterRole(SlotsListModel::DayRole);
        proxy->setFilterFixedString(QString::number(i));

        proxy->setSortRole(SlotsListModel::StartTimeRole);
        proxy->sort(0, Qt::AscendingOrder);

        m_dayProxies.append(proxy);
    }
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
    fetchHours();
}

// returns gym's slots
void GymModelView::selectGym(const QString &gymId) {
    m_selectedGymId = gymId;
    emit selectedGymIdChanged();
    loadSlots(m_selectedGymId);
}

void GymModelView::loadSlots(const QString &gymId) {
    beginRequest();
    m_slotModel->fetchSlots(gymId);
}

QObject *GymModelView::getDayModel(int day) {
    if (day < 1 || day > 7) {
        return nullptr;
    }
    return m_dayProxies[day - 1];
}

bool GymModelView::isSlotBooked(const QString &slotId) {
    if (slotId.isEmpty()) {
        return false;
    }
    return m_BookedSlotIds.contains(slotId);
}

void GymModelView::bookSlot(const QString &slotId) {
    if (m_BookedSlotIds.contains(slotId)) {
        emit actionError("Вы уже записаны в этот слот!");
        return;
    }

    beginRequest();
    m_slotModel->bookSlot(slotId);
}

void GymModelView::cancelBooking(const QString &slotId) {
    beginRequest();
    m_slotModel->cancelBooking(slotId);
}

void GymModelView::fetchBookedSlotsIds() {
    beginRequest();
    m_slotModel->getBookedSlotsIds();
}

void GymModelView::createSlot(
    const QDateTime &startTime,
    const QDateTime &endTime,
    int capacity
) {
    if (m_selectedGymId.isEmpty()) {
        emit actionError("Не выбран зал для создания слота");
        return;
    }
    beginRequest();
    m_slotModel->createSlot(startTime, endTime, m_selectedGymId, capacity);
}

void GymModelView::removeSlot(const QString &slotId) {
    beginRequest();
    m_slotModel->removeSlot(slotId);
}

void GymModelView::createGym(const QString &name) {
    beginRequest();
    m_gymModel->createGym(name);
}

void GymModelView::addHours(int hours, const QString &userId) {
    beginRequest();
    m_gymModel->addHours(hours, userId);
};

void GymModelView::fetchHours() {
    beginRequest();
    m_gymModel->fetchUserGainedHours();
};

void GymModelView::onSlotCreated(const QJsonObject &data) {
    endRequest();
    emit actionSuccess("Слот успешно создан!");
    loadSlots(selectedGymId());
}

void GymModelView::onGymCreated(const QJsonObject &data) {
    endRequest();
    emit actionSuccess("Зал успешно создан!");
    init();
}

void GymModelView::onSlotRemoved(const QJsonObject &data) {
    endRequest();
    emit actionSuccess("Слот успешно удален!");
    loadSlots(selectedGymId());
}

void GymModelView::onGymsLoaded(const QJsonObject &data) {
    endRequest();

    m_gyms.clear();
    m_gyms = data["sections"].toArray().toVariantList();

    emit gymsChanged();
}

void GymModelView::onSlotsLoaded(const QJsonObject &data) {
    endRequest();

    QJsonArray slotsArray = data["slots"].toArray();
    QList<SlotItem> newList;

    for (const QJsonValue &val : slotsArray) {
        QJsonObject slotObj = val.toObject();

        SlotItem item;
        item.id = slotObj["slotId"].toString();
        item.capacity = slotObj["capacity"].toInt();
        item.participantsCount = slotObj["participantsCount"].toInt();
        item.participants = slotObj["participants"].toArray().toVariantList();

        item.startTime =
            QDateTime::fromString(slotObj["startTime"].toString(), Qt::ISODate);
        item.endTime =
            QDateTime::fromString(slotObj["endTime"].toString(), Qt::ISODate);
        newList.append(item);
    }

    m_slotsListModel->setSlots(newList);
}

void GymModelView::onBookingFinished(const QJsonObject &data) {
    endRequest();
    fetchBookedSlotsIds();
    loadSlots(m_selectedGymId);
    emit bookedSlotsChanged();
}

void GymModelView::onStatsLoaded(const QJsonObject &data) {
    endRequest();
    m_userName = data["name"].toString();
    m_hoursCount = data["visit_count"].toInt();
    m_hoursNeeded = data["visits_needed"].toInt(24);
    emit userInfoChanged();
}

void GymModelView::onBookedSlotsIdsFinished(const QJsonObject &data) {
    endRequest();
    m_BookedSlotIds.clear();
    QJsonArray arr = data["enrollments"].toArray();
    for (const QJsonValue &val : arr) {
        m_BookedSlotIds.insert(val.toString());
    }

    emit bookedSlotsChanged();
};

void GymModelView::onHoursAdded(const QJsonObject &data) {
    endRequest();
    emit actionSuccess("Часы успешно зачислены!");
};

void GymModelView::onHoursLoaded(const QJsonObject &data) {
    endRequest();
    m_hoursCount = data["gainedHours"].toInt();
    emit userInfoChanged();
};

void GymModelView::onApiError(const QString &message) {
    endRequest();
    setError(message);
}
