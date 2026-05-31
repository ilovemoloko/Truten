#include "database.hpp"
#include "slot_manager.hpp"

pqxx::result SlotManager::getSlotInfo(const std::string &id, bool for_update) const {
    std::string query = "SELECT * FROM slots WHERE ID = $1";
    if (for_update) query += " FOR UPDATE";
    auto res = db->execute(query, id);
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

std::vector<crow::json::wvalue> SlotManager::getGymList() const {
    std::vector<crow::json::wvalue> response;
    const auto res = db->execute("SELECT name, id FROM gyms");
    for (auto rw : res) {
        crow::json::wvalue tmp;
        tmp["name"] = rw[0].as<std::string>();
        tmp["id"] = rw[1].as<std::string>();
        response.push_back(std::move(tmp));
    }
    return response;
}

std::string SlotManager::getNameById(const std::string &user_id) const {
    const auto res = db->execute("SELECT name FROM users WHERE id = $1", user_id)[0][0];
    return res.as<std::string>();
}


std::vector<crow::json::wvalue> SlotManager::loadUserList(const pqxx::field& field) const {
    std::vector<crow::json::wvalue> participants;
    auto enrolled_array = field.as_array();
    std::pair<pqxx::array_parser::juncture, std::string> elem;
    do {
        elem = enrolled_array.get_next();
        if (elem.first == pqxx::array_parser::juncture::string_value) {
            crow::json::wvalue tmp;
            const std::string id = elem.second;
            tmp["userId"] = id;
            tmp["userName"] = getNameById(id);
            participants.push_back(std::move(tmp));
        }
    } while (elem.first != pqxx::array_parser::juncture::done);
    return participants;
}

std::vector<crow::json::wvalue> SlotManager::getGymSlots(const std::string &gym_id) const {
    std::vector<crow::json::wvalue> response;
    const auto res = db->execute("SELECT ID FROM slots WHERE section_id = $1",
                                 gym_id);
    for (auto rw : res) {
        crow::json::wvalue to_push;
        const auto slot_id = rw[0].as<std::string>();
        to_push = getSlotInfoJSON(slot_id);
        response.push_back(std::move(to_push));
    }
    return response;
}

crow::json::wvalue SlotManager::getSlotInfoJSON(const std::string& slot_id, bool for_update) const {
    auto res = getSlotInfo(slot_id, for_update)[0];
    crow::json::wvalue to_return;
    auto participants = loadUserList(res[4]);
    auto queue_users = loadUserList(res[8]);
    //kill me
    to_return["slotId"] = res[0].as<std::string>();
    to_return["gymId"] = res[1].as<std::string>();
    to_return["gymName"] = res[2].as<std::string>();
    to_return["capacity"] = res[3].as<short>();
    to_return["participantsCount"] = static_cast<short>(participants.size());
    to_return["participants"] = std::move(participants);
    to_return["startTime"] = res[5].as<std::string>();
    to_return["endTime"] = res[6].as<std::string>();
    to_return["queueCount"] = static_cast<short>(queue_users.size());
    to_return["queue"] = std::move(queue_users);
    return to_return;
}

void SlotManager::addAdminToGym(const std::string gym_id, const std::string &new_admin) {
    db->execute("UPDATE gyms SET admins = array_append(admins, $1) WHERE id = $2", new_admin, gym_id);
}

void SlotManager::createGym(const std::string &gym_name, const std::string &creator_id) {
    auto res = db->execute("INSERT INTO gyms(name) VALUES ($1) RETURNING id", gym_name);
    auto gym_id = res[0][0].as<std::string>();
    addAdminToGym(gym_id, creator_id);
}


std::vector<crow::json::wvalue> SlotManager::getGymAdmins(const std::string &gym_id) const {
    auto admins = db->execute("SELECT admins FROM gyms WHERE id = $1", gym_id)[0][0];
    std::vector<crow::json::wvalue> to_return = loadUserList(admins);
    return to_return;
}


void SlotManager::createSlot(const std::string &gym_id, std::string &time_begin, std::string &time_end, const int capacity) {
    auto gym_name_q = db->execute("SELECT name FROM gyms WHERE id = $1", gym_id)[0][0];
    auto gym_name = gym_name_q.as<std::string>();
    db->execute("INSERT INTO slots(section_id, section_name, start_time, end_time, capacity) VALUES($1, $2, $3, $4, $5)", gym_id, gym_name, time_begin, time_end, capacity);
}

void SlotManager::deleteSlot(const std::string &slot_id) {
    auto slot_info = getSlotInfo(slot_id, true)[0];
    auto participants = loadUserList(slot_info[4]);
    for (auto &part : participants) {
        std::string user_id = part["userId"].dump();
        //sadly .dump() leaves the string in a weird format like ""a""
        user_id.pop_back();
        user_id.erase(user_id.begin());
        db->execute("UPDATE users SET enrolled_slots = array_remove(enrolled_slots, $1) WHERE ID = $2", slot_id, user_id);
    }
    auto queue_users = loadUserList(slot_info[8]);
    for (auto &part : queue_users) {
        std::string user_id = part["userId"].dump();
        user_id.pop_back();
        user_id.erase(user_id.begin());
        db->execute("UPDATE users SET queued_slots = array_remove(queued_slots, $1) WHERE ID = $2", slot_id, user_id);
    }
    db->execute("DELETE FROM slots WHERE id = $1", slot_id);
}

bool SlotManager::isAtCapacity(const std::string &slot_id, bool for_update) const {
    std::string query = "SELECT capacity, COALESCE(array_length(enrolled, 1), 0) FROM slots WHERE id = $1";
    if (for_update) query += " FOR UPDATE";
    auto res = db->execute(query, slot_id)[0];
    return res[1].as<int>() >= res[0].as<int>();
}

void SlotManager::addToQueue(const std::string &user_id, const std::string &slot_id) {
    db->execute("UPDATE slots SET queue = array_append(queue, $1) WHERE id = $2", user_id, slot_id);
}

void SlotManager::removeFromQueue(const std::string &user_id, const std::string &slot_id) {
    db->execute("UPDATE slots SET queue = array_remove(queue, $1) WHERE id = $2", user_id, slot_id);
}

std::string SlotManager::getFirstInQueue(const std::string &slot_id, bool for_update) const {
    std::string query = "SELECT queue[1] FROM slots WHERE id = $1";
    if (for_update) query += " FOR UPDATE";
    auto res = db->execute(query, slot_id)[0][0];
    if (res.is_null()) return "";
    return res.as<std::string>();
}

std::vector<std::string> SlotManager::getQueueUserIds(const std::string &slot_id) const {
    auto field = db->execute("SELECT queue FROM slots WHERE id = $1", slot_id)[0][0];
    std::vector<std::string> result;
    if (field.is_null()) return result;
    auto arr = field.as_array();
    std::pair<pqxx::array_parser::juncture, std::string> elem;
    do {
        elem = arr.get_next();
        if (elem.first == pqxx::array_parser::juncture::string_value) {
            result.push_back(elem.second);
        }
    } while (elem.first != pqxx::array_parser::juncture::done);
    return result;
}
