#ifndef GYMMODEL_H
#define GYMMODEL_H

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include "BaseAPIModel.h"

class GymModel : public BaseModel {
    Q_OBJECT

public:
    explicit GymModel(QNetworkAccessManager *manager, QObject *parent = nullptr);

    void fetchGyms();
    void fetchSlots(int gymId);
    void bookSlot(int slotId);
    void cancelBooking(int slotId);
    void fetchUserStats();

signals:
    void gymsLoaded(const QJsonObject &data);
    void slotsLoaded(const QJsonObject &data);
    void bookingFinished(const QJsonObject &data);
    void statsLoaded(const QJsonObject &data);
    void apiError(const QString &message);

private:
    void handleReply(
        QNetworkReply *reply,
        std::function<void(int, const QJsonObject &)> onSuccess
    );
};

#endif  // GYMMODEL_H
