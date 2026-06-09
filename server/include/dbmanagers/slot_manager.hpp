#ifndef TRUTEN_SERVER_SLOT_MANAGER_HPP
#define TRUTEN_SERVER_SLOT_MANAGER_HPP
#include "database.hpp"
#include <vector>
#include "crow.h"

// I suppose you can guess what it does by the name. Come on. We're not
// 5-year-olds here. Go on.
// We are:)
struct SlotManager {
public:
    explicit SlotManager(std::shared_ptr<Database> database)
        : db(std::move(database)) {
    }

    void changeSlotInfo(const std::string &id, const std::string &start_time,
                        const std::string &end_time, int capacity);

    void closeSlot(const std::string &id);

    void removeEntry(const std::string &user_id, const std::string &slot_id);

    void addEntry(const std::string &user_id, const std::string &slot_id);

    [[nodiscard]] pqxx::result getSlotInfo(const std::string &id, bool for_update = false) const;

    [[nodiscard]] std::vector<crow::json::wvalue> getGymList() const;

    [[nodiscard]] std::vector<crow::json::wvalue> loadUserList(const pqxx::row::reference & field) const;

    [[nodiscard]] std::vector<crow::json::wvalue> getGymSlots(const std::string& gym_id) const;

    [[nodiscard]] crow::json::wvalue getSlotInfoJSON(const std::string& slot_id, bool for_update = false) const;

    void createGym(const std::string& gym_name, const std::string& creator_id);

    void createSlot(const std::string& gym_id, std::string& time_begin, std::string& time_end, const int capacity);

    void deleteSlot(const std::string& slot_id);

    void addAdminToGym(const std::string gym_id, const std::string& new_admin);

    [[nodiscard]] std::vector<crow::json::wvalue> getGymAdmins(const std::string& gym_id) const;

    std::string getNameById(const std::string& user_id) const;

    void lockSlot(const std::string& slot_id) const;

    [[nodiscard]] bool isAtCapacity(const std::string& slot_id) const;

    void addToQueue(const std::string& user_id, const std::string& slot_id);

    void removeFromQueue(const std::string& user_id, const std::string& slot_id);

    [[nodiscard]] std::string getFirstInQueue(const std::string& slot_id) const;

    [[nodiscard]] std::vector<std::string> getQueueUserIds(const std::string& slot_id) const;

    void resetWeekly();

private:
    std::shared_ptr<Database> db;
};

#endif //TRUTEN_SERVER_SLOT_MANAGER_HPP