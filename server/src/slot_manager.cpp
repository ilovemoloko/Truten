#include "database.hpp"
#include "slot_manager.hpp"

pqxx::result SlotManager::getSlotInfo(const std::string &id) const {
    auto res = db->execute("SELECT * FROM slots WHERE ID = $1", id);
    return res;
}

void SlotManager::changeSlotInfo(const std::string &id,
                                 const std::string &start_time,
                                 const std::string &end_time, int capacity) {
    db->execute("UPDATE slots SET capacity = $1, start_time = $2, end_time = "
                    "$3 WHERE ID = $4",
                    capacity, start_time, end_time, id);
}

void SlotManager::removeEntry(const std::string &user_id,
                              const std::string &slot_id) {
    db->execute(
        "UPDATE slots SET enrolled = array_remove(enrolled, $1) WHERE ID = $2",
        user_id, slot_id);
}

void SlotManager::addEntry(const std::string &user_id,
                           const std::string &slot_id) {
    db->execute(
        "UPDATE slots SET enrolled = array_append(enrolled, $1) WHERE ID = $2",
        user_id, slot_id);
}

void SlotManager::closeSlot(const std::string &id) {
    db->execute("UPDATE slots SET is_cancelled = TRUE WHERE ID = $1", id);
}

GymList SlotManager::getGymList() const {
    GymList response;
    const auto res = db->execute("SELECT name, id FROM gyms");
    for (auto i : res) {
        response.gyms.push_back(i[0].as<std::string>());
        response.ids.push_back(i[1].as<std::string>());
    }
    return response;
}