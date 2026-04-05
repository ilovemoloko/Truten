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

signals:
    void slotsLoaded(const QJsonObject &data);
    void bookingFinished(const QJsonObject &data);
    void slotError(const QString &message);
};

#endif  // SLOTMODEL_H
