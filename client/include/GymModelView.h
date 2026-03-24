#ifndef GYMMODELVIEW_H
#define GYMMODELVIEW_H

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QVariantList>
#include "GymModel.h"

class GymModelView : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(QVariantList gyms READ gyms NOTIFY gymsChanged)
    Q_PROPERTY(QVariantList slots READ slots NOTIFY slotsChanged)
    Q_PROPERTY(int selectedGymId READ selectedGymId NOTIFY selectedGymIdChanged)
    Q_PROPERTY(QString userName READ userName NOTIFY userInfoChanged)
    Q_PROPERTY(int visitCount READ visitCount NOTIFY userInfoChanged)
    Q_PROPERTY(int visitsNeeded READ visitsNeeded NOTIFY userInfoChanged)

public:
    explicit GymModelView(GymModel *model, QObject *parent = nullptr);

    bool isLoading() const {
        return m_isLoading;
    }
    QString errorMessage() const {
        return m_errorMessage;
    }
    QVariantList gyms() const {
        return m_gyms;
    }
    QVariantList slots() const {
        return m_slots;
    }
    int selectedGymId() const {
        return m_selectedGymId;
    }
    QString userName() const {
        return m_userName;
    }
    int visitCount() const {
        return m_visitCount;
    }
    int visitsNeeded() const {
        return m_visitsNeeded;
    }

    Q_INVOKABLE void init();
    Q_INVOKABLE void selectGym(int gymId);
    Q_INVOKABLE void bookSlot(int slotId);
    Q_INVOKABLE void cancelBooking(int slotId);
    Q_INVOKABLE void reloadSlots();

signals:
    void isLoadingChanged();
    void errorMessageChanged();
    void gymsChanged();
    void slotsChanged();
    void selectedGymIdChanged();
    void userInfoChanged();
    void actionSuccess(const QString &message);
    void actionError(const QString &message);

private slots:
    void onGymsLoaded(const QJsonObject &data);
    void onSlotsLoaded(const QJsonObject &data);
    void onBookingFinished(const QJsonObject &data);
    void onStatsLoaded(const QJsonObject &data);
    void onApiError(const QString &message);

private:
    GymModel *m_model;
    bool m_isLoading = false;
    QString m_errorMessage;
    QVariantList m_gyms;
    QVariantList m_slots;
    int m_selectedGymId = -1;
    QString m_userName;
    int m_visitCount = 0;
    int m_visitsNeeded = 8;
    int m_pendingRequests = 0;

    void setLoading(bool loading);
    void setError(const QString &error);
    void beginRequest();
    void endRequest();
};

#endif  // GYMMODELVIEW_H
