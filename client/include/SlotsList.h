#ifndef SLOTSLIST_H
#define SLOTSLIST_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QVariantList>

struct SlotItem {
    QString id;
    int capacity;
    int participantsCount;
    QVariantList participants;
    int queueCount;
    QVariantList queue;
    QDateTime startTime;
    QDateTime endTime;
};

class SlotsListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        CapacityRole,
        ParticipantsCountRole,
        ParticipantsRole,
        QueueCountRole,
        QueueRole,
        StartTimeRole,
        EndTimeRole,
        DayRole
    };

    explicit SlotsListModel(QObject *parent = nullptr)
        : QAbstractListModel(parent) {
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return m_items.size();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole)
        const override {
        if (!index.isValid() || index.row() >= m_items.size()) {
            return {};
        }

        const auto &item = m_items[index.row()];

        switch (role) {
            case IdRole:
                return item.id;
            case CapacityRole:
                return item.capacity;
            case ParticipantsCountRole:
                return item.participantsCount;
            case ParticipantsRole:
                return item.participants;
            case QueueCountRole:
                return item.queueCount;
            case QueueRole:
                return item.queue;
            case StartTimeRole:
                return item.startTime;
            case EndTimeRole:
                return item.endTime;
            case DayRole:
                return item.startTime.date().dayOfWeek(
                );  // return 1 monday ... 7 sunday
            default:
                return {};
        }
    }

    void setSlots(const QList<SlotItem> &items) {
        beginResetModel();
        m_items = items;
        endResetModel();
    }

    QHash<int, QByteArray> roleNames() const override {
        return {
            {IdRole, "slotId"},
            {CapacityRole, "capacity"},
            {ParticipantsCountRole, "participantsCount"},
            {ParticipantsRole, "participants"},
            {QueueCountRole, "queueCount"},
            {QueueRole, "slotQueue"},
            {StartTimeRole, "startTime"},
            {EndTimeRole, "endTime"},
            {DayRole, "dayOfWeek"}};
    }

private:
    QList<SlotItem> m_items;
};

#endif  // SLOTSLIST_H
