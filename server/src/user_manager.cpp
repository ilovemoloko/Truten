#include "database.hpp"
#include "user_manager.hpp"

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