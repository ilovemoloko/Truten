#ifndef GYMMODELVIEW_H
#define GYMMODELVIEW_H

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QVariantList>
#include "GymModel.h"
#include "SlotModel.h"
#include "SlotsList.h"

class GymModelView : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage WRITE setError NOTIFY
                   errorMessageChanged)
    Q_PROPERTY(QVariantList gyms READ gyms NOTIFY gymsChanged)
    Q_PROPERTY(QString selectedGymId READ selectedGymId NOTIFY
                   selectedGymIdChanged)
    Q_PROPERTY(QString userName READ userName NOTIFY userInfoChanged)
    Q_PROPERTY(int hoursCount READ hoursCount NOTIFY userInfoChanged)
    Q_PROPERTY(int hoursNeeded READ hoursNeeded NOTIFY userInfoChanged)
    Q_PROPERTY(bool isAdmin READ isAdmin NOTIFY userInfoChanged)
    Q_PROPERTY(QVariantList gymStatsList READ gymStatsList NOTIFY gymStatsChanged)

public:
    explicit GymModelView(
        GymModel *gymModel,
        SlotModel *slotModel,
        QObject *parent = nullptr
    );

    bool isLoading() const {
        return m_isLoading;
    };

    QString errorMessage() const {
        return m_errorMessage;
    };

    void setErrorMessage(const QString &err_message) {
        m_errorMessage = err_message;
    }

    QVariantList gyms() const {
        return m_gyms;
    };

    QString selectedGymId() const {
        return m_selectedGymId;
    };

    QString userName() const {
        return m_userName;
    };

    int hoursCount() const {
        return m_hoursCount;
    };

    int hoursNeeded() const {
        return m_hoursNeeded;
    };

    bool isAdmin() {
        return m_gymModel->isAdmin();
    }

    QVariantList gymStatsList() const {
        return m_gymStatsList;
    }

    Q_INVOKABLE void init();
    Q_INVOKABLE void selectGym(const QString &gymId);
    Q_INVOKABLE void bookSlot(const QString &slotId);
    Q_INVOKABLE void cancelBooking(const QString &slotId);
    Q_INVOKABLE void loadSlots(const QString &gymId);
    Q_INVOKABLE QObject *getDayModel(int day);
    Q_INVOKABLE bool isSlotBooked(const QString &slotId);
    Q_INVOKABLE void createSlot(
        const QDateTime &startTime,
        const QDateTime &endTime,
        int capacity
    );
    Q_INVOKABLE void removeSlot(const QString &slotId);
    Q_INVOKABLE void createGym(const QString &name);
    Q_INVOKABLE void fetchBookedSlotsIds();
    Q_INVOKABLE void addHours(int hours, const QString &userId);
    Q_INVOKABLE void fetchHours();
    Q_INVOKABLE void joinQueue(const QString &slotId);
    Q_INVOKABLE void leaveQueue(const QString &slotId);
    Q_INVOKABLE bool isSlotQueued(const QString &slotId);
    Q_INVOKABLE void fetchQueuedSlotsIds();
    Q_INVOKABLE void addGymAdmin(const QString &userId);
    Q_INVOKABLE void fetchStats();
    Q_INVOKABLE void clearUserState();

signals:
    void isLoadingChanged();
    void errorMessageChanged();
    void gymsChanged();
    void slotsChanged();
    void selectedGymIdChanged();
    void userInfoChanged();
    void actionSuccess(const QString &message);
    void actionError(const QString &message);
    void bookedSlotsChanged();
    void queuedSlotsChanged();
    void gymStatsChanged();

private slots:
    void onGymsLoaded(const QJsonObject &data);
    void onSlotsLoaded(const QJsonObject &data);
    void onBookingFinished(const QJsonObject &data);
    void onStatsLoaded(const QJsonObject &data);
    void onApiError(const QString &message);
    void onSlotCreated(const QJsonObject &data);
    void onSlotRemoved(const QJsonObject &data);
    void onGymCreated(const QJsonObject &data);
    void onBookedSlotsIdsFinished(const QJsonObject &data);
    void onHoursLoaded(const QJsonObject &data);
    void onHoursAdded(const QJsonObject &data);
    void onQueueJoined(const QJsonObject &data);
    void onQueueLeft(const QJsonObject &data);
    void onQueuedSlotsIdsFinished(const QJsonObject &data);
    void onGymAdminAdded(const QJsonObject &data);

private:
    GymModel *m_gymModel;
    SlotModel *m_slotModel;
    SlotsListModel *m_slotsListModel;

    bool m_isLoading = false;
    QString m_errorMessage;
    QVariantList m_gyms;
    // contains slots for each day of week for selected gym
    QList<QSortFilterProxyModel *> m_dayProxies;
    QString m_selectedGymId;
    QSet<QString> m_BookedSlotIds;
    QSet<QString> m_QueuedSlotIds;
    QMap<int, QString> m_bookingsByDay;
    QVariantList m_gymStatsList;
    QString m_userName;

    int m_hoursCount = 0;
    int m_hoursNeeded = 24;
    int m_pendingRequests = 0;
    int m_pendingActionRequests = 0;

    void setLoading(bool loading);
    void setError(const QString &error);
    void beginRequest();
    void endRequest();
    void beginActionRequest();
    void endActionRequest();
};

#endif  // GYMMODELVIEW_H
