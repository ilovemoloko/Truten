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

signals:
    void gymsLoaded(const QJsonObject &data);
    void statsLoaded(const QJsonObject &data);
    void gymError(const QString &message);
};

#endif  // GYMMODEL_H
