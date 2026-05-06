#ifndef SLOTMODEL_H
#define SLOTMODEL_H
#include "BaseAPIModel.h"

class SlotModel : public BaseModel {
    Q_OBJECT

public:
    explicit SlotModel(
        QNetworkAccessManager *manager,
        QObject *parent = nullptr
    );

    void fetchSlots(const QString &gymId);
    void bookSlot(const QString &slotId);
    void cancelBooking(const QString &slotId);
    void removeSlot(const QString &slotId);
    void createSlot(
        const QDateTime &startTime,
        const QDateTime &endTime,
        const QString &gymId,
        int capacity
    );
    void getBookedSlotsIds(const QString &userId = getUserId());
    //add queue methods
    void joinQueue(const QString &slotId);
    void leaveQueue(const QString &slotId);
    void getQueuedSlotsIds(const QString &userId = getUserId());

signals:
    void slotsLoaded(const QJsonObject &data);
    void bookingFinished(const QJsonObject &data);
    void slotError(const QString &message);
    void slotCreated(const QJsonObject &data);
    void slotRemoved(const QJsonObject &data);
    void bookedSlotsIdsFinished(const QJsonObject &data);
    //new signals for queue
    void queueJoinFinished(const QJsonObject &data);
    void queueLeaveFinished(const QJsonObject &data);
    void queuedSlotsIdsFinished(const QJsonObject &data);
};

#endif  // SLOTMODEL_H
