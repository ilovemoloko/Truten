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


std::vector<crow::json::wvalue> SlotManager::loadParticipants(const pqxx::field& field) const {
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
    const auto res = db->execute("SELECT ID, start_time, end_time, capacity, enrolled FROM slots WHERE section_id = $1",
                                 gym_id);
    for (auto rw : res) {
        crow::json::wvalue to_push;
        std::vector<crow::json::wvalue> participants = loadParticipants(rw[4]);
        to_push["slotId"] = rw[0].as<std::string>();
        to_push["startTime"] = rw[1].as<std::string>();
        to_push["endTime"] = rw[2].as<std::string>();
        to_push["capacity"] = rw[3].as<short>();
        to_push["participants"] = std::move(participants);
        response.push_back(std::move(to_push));
    }
    return response;
}

crow::json::wvalue SlotManager::getSlotInfoJSON(const std::string& slot_id) const {
    auto res = getSlotInfo(slot_id)[0];
    crow::json::wvalue to_return;
    auto participants = loadParticipants(res[4]);
    //kill me
    to_return["slotId"] = res[0].as<std::string>();
    to_return["gymId"] = res[1].as<std::string>();
    to_return["gymName"] = res[2].as<std::string>();
    to_return["capacity"] = res[3].as<short>();
    to_return["participantsCount"] = static_cast<short>(participants.size());
    to_return["participants"] = std::move(participants);
    to_return["startTime"] = res[5].as<std::string>();
    to_return["endTime"] = res[6].as<std::string>();
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
    std::vector<crow::json::wvalue> to_return;
    auto admins = db->execute("SELECT admins FROM gyms WHERE id = $1", gym_id)[0][0];
    auto admins_array = admins.as_array();
    std::pair<pqxx::array_parser::juncture, std::string> elem;
    do {
        elem = admins_array.get_next();
        if (elem.first == pqxx::array_parser::juncture::string_value) {
            crow::json::wvalue tmp;
            tmp["userId"] = elem.second;
            tmp["userName"] = getNameById(elem.second);
            to_return.push_back(std::move(tmp));
        }
    } while (elem.first != pqxx::array_parser::juncture::done);
    return to_return;
}


void SlotManager::createSlot(const std::string &gym_id, std::string &time_begin, std::string &time_end, const int capacity) {
    auto gym_name_q = db->execute("SELECT name FROM gyms WHERE id = $1", gym_id)[0][0];
    auto gym_name = gym_name_q.as<std::string>();
    db->execute("INSERT INTO SLOTS(section_id, section_name, start_time, end_time, capacity) VALUES($1, $2, $3, $4, $5)", gym_id, gym_name, time_begin, time_end, capacity);
}
