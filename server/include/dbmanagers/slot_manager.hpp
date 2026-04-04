#ifndef TRUTEN_SERVER_SLOT_MANAGER_HPP
#define TRUTEN_SERVER_SLOT_MANAGER_HPP
#include "database.hpp"


//it's only ever used in "get gym list"
//i guess it makes sense for this struct to
//be here
struct GymList {
    std::vector<std::string> gyms, ids;
};


// I suppose you can guess what it does by the name. Come on. We're not
// 5-year-olds here. Go on.
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

    [[nodiscard]] pqxx::result getSlotInfo(const std::string &id) const;

    [[nodiscard]] GymList getGymList() const;
private:
    std::shared_ptr<Database> db;
};

#endif //TRUTEN_SERVER_SLOT_MANAGER_HPP