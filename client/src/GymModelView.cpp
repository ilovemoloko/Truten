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
    connect(
        m_gymModel, &GymModel::gymAdminAdded, this,
        &GymModelView::onGymAdminAdded
    );
    connect(m_slotModel, &SlotModel::queueJoinFinished, this, &GymModelView::onQueueJoined);
    connect(m_slotModel, &SlotModel::queueLeaveFinished, this, &GymModelView::onQueueLeft);
    connect(m_slotModel, &SlotModel::queuedSlotsIdsFinished, this, &GymModelView::onQueuedSlotsIdsFinished);

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
}

void GymModelView::beginActionRequest() {
    m_pendingActionRequests++;
    beginRequest();
}

void GymModelView::endActionRequest() {
    m_pendingActionRequests = qMax(0, m_pendingActionRequests - 1);
    endRequest();
}

void GymModelView::init() {
    m_errorMessage.clear();
    emit errorMessageChanged();

    beginRequest();
    m_gymModel->fetchGyms();
    fetchHours();
    fetchQueuedSlotsIds();
}

// returns gym's slots
void GymModelView::selectGym(const QString &gymId) {
    m_selectedGymId = gymId;
    emit selectedGymIdChanged();
    loadSlots(m_selectedGymId);
}
//add queue info
void GymModelView::loadSlots(const QString &gymId) {
    beginRequest();
    m_slotModel->fetchSlots(gymId);
    fetchBookedSlotsIds();
    fetchQueuedSlotsIds();
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

    beginActionRequest();
    m_slotModel->bookSlot(slotId);
}

void GymModelView::cancelBooking(const QString &slotId) {
    beginActionRequest();
    m_slotModel->cancelBooking(slotId);
}

void GymModelView::fetchBookedSlotsIds() {
    beginRequest();
    m_slotModel->getBookedSlotsIds();
}

void GymModelView::fetchQueuedSlotsIds() {
    beginRequest();
    m_slotModel->getQueuedSlotsIds();
}

void GymModelView::joinQueue(const QString &slotId) {
    if (m_QueuedSlotIds.contains(slotId)) {
        emit actionError("Вы уже в очереди на этот слот!");
        return;
    }
    beginRequest();
    m_slotModel->joinQueue(slotId);
}

void GymModelView::leaveQueue(const QString &slotId) {
    beginRequest();
    m_slotModel->leaveQueue(slotId);
}

bool GymModelView::isSlotQueued(const QString &slotId) {
    if (slotId.isEmpty()) return false;
    return m_QueuedSlotIds.contains(slotId);
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
    beginActionRequest();
    m_slotModel->createSlot(startTime, endTime, m_selectedGymId, capacity);
}

void GymModelView::removeSlot(const QString &slotId) {
    beginActionRequest();
    m_slotModel->removeSlot(slotId);
}

void GymModelView::createGym(const QString &name) {
    beginActionRequest();
    m_gymModel->createGym(name);
}

void GymModelView::addHours(int hours, const QString &userId) {
    beginActionRequest();
    m_gymModel->addHours(hours, userId);
};

void GymModelView::fetchHours() {
    beginRequest();
    m_gymModel->fetchUserGainedHours();
};

void GymModelView::onSlotCreated(const QJsonObject &data) {
    endActionRequest();
    emit actionSuccess("Слот успешно создан!");
    loadSlots(selectedGymId());
}

void GymModelView::onGymCreated(const QJsonObject &data) {
    endActionRequest();
    emit actionSuccess("Зал успешно создан!");
    init();
}

void GymModelView::onSlotRemoved(const QJsonObject &data) {
    endActionRequest();
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
        item.queueCount = slotObj["queueCount"].toInt();
        item.queue = slotObj["queue"].toArray().toVariantList();

        item.startTime =
            QDateTime::fromString(slotObj["startTime"].toString(), Qt::ISODate);
        item.endTime =
            QDateTime::fromString(slotObj["endTime"].toString(), Qt::ISODate);
        newList.append(item);
    }

    m_slotsListModel->setSlots(newList);
}

void GymModelView::onBookingFinished(const QJsonObject &data) {
    endActionRequest();
    fetchBookedSlotsIds();
    loadSlots(m_selectedGymId);
    emit bookedSlotsChanged();
}

void GymModelView::onStatsLoaded(const QJsonObject &data) {
    endRequest();
    m_gymStatsList.clear();

    QJsonObject gymsObj = data["gyms"].toObject();
    for (auto it = gymsObj.begin(); it != gymsObj.end(); ++it) {
        QString gymId = it.key();
        int hours = it.value().toInt();

        QString gymName = gymId;
        for (const QVariant &gymVar : m_gyms) {
            QVariantMap gym = gymVar.toMap();
            if (gym["id"].toString() == gymId) {
                gymName = gym["name"].toString();
                break;
            }
        }

        QVariantMap entry;
        entry["name"] = gymName;
        entry["hours"] = hours;
        m_gymStatsList.append(entry);
    }

    emit gymStatsChanged();
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
    endActionRequest();
    emit actionSuccess("Часы успешно зачислены!");
};

void GymModelView::onQueueJoined(const QJsonObject &data) {
    endRequest();
    // server may have enrolled directly if slot freed up
    if (data["enrolled"].toBool(false)) {
        emit actionSuccess("Место освободилось — вы записаны!");
    } else {
        emit actionSuccess("Вы встали в очередь!");
    }
    fetchQueuedSlotsIds();
    fetchBookedSlotsIds();
    loadSlots(m_selectedGymId);
}

void GymModelView::onQueueLeft(const QJsonObject &data) {
    endRequest();
    emit actionSuccess("Вы вышли из очереди!");
    fetchQueuedSlotsIds();
    loadSlots(m_selectedGymId);
}

void GymModelView::onQueuedSlotsIdsFinished(const QJsonObject &data) {
    endRequest();
    m_QueuedSlotIds.clear();
    QJsonArray arr = data["queuedSlots"].toArray();
    for (const QJsonValue &val : arr) {
        m_QueuedSlotIds.insert(val.toString());
    }
    emit bookedSlotsChanged();
}

void GymModelView::onHoursLoaded(const QJsonObject &data) {
    endRequest();
    m_hoursCount = data["gainedHours"].toInt();
    emit userInfoChanged();
};

void GymModelView::addGymAdmin(const QString &userId) {
    if (m_selectedGymId.isEmpty()) {
        emit actionError("Не выбран зал");
        return;
    }
    beginActionRequest();
    m_gymModel->addGymAdmin(m_selectedGymId, userId);
}

void GymModelView::fetchStats() {
    beginRequest();
    m_gymModel->fetchUserStats();
}

void GymModelView::clearUserState() {
    m_BookedSlotIds.clear();
    m_QueuedSlotIds.clear();
    m_gyms.clear();
    m_gymStatsList.clear();
    m_slotsListModel->setSlots({});
    m_selectedGymId.clear();
    emit gymsChanged();
    emit bookedSlotsChanged();
    emit queuedSlotsChanged();
    emit gymStatsChanged();
}

void GymModelView::onGymAdminAdded(const QJsonObject &data) {
    endActionRequest();
    emit actionSuccess("Администратор успешно добавлен!");
}

void GymModelView::onApiError(const QString &message) {
    if (m_pendingActionRequests > 0) {
        endActionRequest();
        emit actionError(message);
    } else {
        endRequest();
        setError(message);
    }
}
