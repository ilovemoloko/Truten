#include "database.hpp"
#include "user_manager.hpp"
#include "crow.h"

void UserManager::addUserHours(const std::string &id, int amount) {
    db->execute("UPDATE users SET hours = hours + $1 WHERE ID = $2", amount,
                    id);
}

void UserManager::banUser(const std::string &id, int duration) {
    int new_time = static_cast<int>(time(nullptr)) + duration * 60 * 60;
    db->execute("UPDATE users SET unban_time = unban_time + $1 WHERE ID = $2",
                    new_time, id);
}

bool UserManager::isAdmin(const std::string &id) const {
    const auto resp =
            db->execute("SELECT is_admin FROM users WHERE ID = $1", id);
    return resp.begin()[0].as<bool>();
}

bool UserManager::userExists(const std::string &id) const {
    const auto resp = db->execute("SELECT * FROM users WHERE ID = $1", id);
    return !resp.empty();
}

int UserManager::getUnbanTime(const std::string &id) const {
    const auto resp = db->execute("SELECT unbanTime FROM users WHERE ID = $1", id);
    return resp.begin()[0].as<int>();
}

int UserManager::getUserHours(const std::string &id) const {
    const auto resp = db->execute("SELECT hours FROM users WHERE ID = $1", id);
    return resp.begin()[0].as<int>();
}

void UserManager::deleteUser(const std::string &id) {
    db->execute("DELETE FROM users WHERE id = $1", id);
}

std::vector<std::string> UserManager::getUserEnrollments(const std::string& user_id) const {
    const auto resp = db->execute("SELECT enrolled_slots FROM users WHERE ID = $1", user_id)[0][0];
    auto enrolled_array = resp.as_array();
    std::vector<std::string> to_return;
    std::pair<pqxx::array_parser::juncture, std::string> elem;
    do {
        elem = enrolled_array.get_next();
        if (elem.first == pqxx::array_parser::juncture::string_value) {
            to_return.push_back(elem.second);
        }
    } while (elem.first != pqxx::array_parser::juncture::done);
    return to_return;
}

void UserManager::addEnrollment(const std::string &user_id, const std::string &slot_id) {
    db->execute("UPDATE users SET enrolled_slots = array_append(enrolled_slots, $1) WHERE ID = $2", slot_id, user_id);
}

void UserManager::removeEnrollment(const std::string &user_id, const std::string &slot_id) {
    db->execute("UPDATE users SET enrolled_slots = array_remove(enrolled_slots, $1) WHERE ID = $2", slot_id, user_id);
}

std::string UserManager::getNameById(const std::string &user_id) const {
    const auto res = db->execute("SELECT name FROM users WHERE id = $1", user_id)[0][0];
    return res.as<std::string>();
}

void UserManager::addQueuedSlot(const std::string &user_id, const std::string &slot_id) {
    db->execute("UPDATE users SET queued_slots = array_append(queued_slots, $1) WHERE ID = $2", slot_id, user_id);
}

void UserManager::removeQueuedSlot(const std::string &user_id, const std::string &slot_id) {
    db->execute("UPDATE users SET queued_slots = array_remove(queued_slots, $1) WHERE ID = $2", slot_id, user_id);
}

std::vector<std::string> UserManager::getUserQueuedSlots(const std::string &user_id) const {
    const auto resp = db->execute("SELECT queued_slots FROM users WHERE ID = $1", user_id)[0][0];
    auto arr = resp.as_array();
    std::vector<std::string> to_return;
    std::pair<pqxx::array_parser::juncture, std::string> elem;
    do {
        elem = arr.get_next();
        if (elem.first == pqxx::array_parser::juncture::string_value) {
            to_return.push_back(elem.second);
        }
    } while (elem.first != pqxx::array_parser::juncture::done);
    return to_return;
}
