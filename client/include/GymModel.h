#ifndef GYMMODEL_H
#define GYMMODEL_H

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include "BaseAPIModel.h"

class GymModel : public BaseModel {
    Q_OBJECT

public:
    explicit GymModel(
        QNetworkAccessManager *manager,
        QObject *parent = nullptr
    );

    void fetchGyms();
    void fetchUserStats(const QString &userId = getUserId());
    void createGym(const QString &name);
    void fetchUserGainedHours(const QString &userId = getUserId());
    void addHours(int hours, const QString &userId = getUserId());
    void addGymAdmin(const QString &gymId, const QString &userId);

signals:
    void gymsLoaded(const QJsonObject &data);
    void statsLoaded(const QJsonObject &data);
    void gymError(const QString &message);
    void gymCreated(const QJsonObject &data);
    void gymRemoved(const QJsonObject &data);
    void hoursLoaded(const QJsonObject &data);
    void hoursAdded(const QJsonObject &data);
    void gymAdminAdded(const QJsonObject &data);
};

#endif  // GYMMODEL_H
