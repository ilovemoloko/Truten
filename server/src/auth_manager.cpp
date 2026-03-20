#include "database.hpp"
#include "auth_manager.hpp"

bool AuthManager::emailExists(const std::string &email) const {
    const auto res = db->execute("SELECT 1 FROM users WHERE email = $1", email);
    return !res.empty();
}

std::string AuthManager::getUserIdByEmail(const std::string &email) const {
    const auto res = db->execute("SELECT id FROM users WHERE email = $1", email);
    if (res.empty()) {
        return "";
    }
    return res[0]["id"].as<std::string>();
}

std::string AuthManager::getPasswordByEmail(const std::string &email) const {
    const auto res = db->execute(
        "SELECT password_hash FROM users WHERE email = $1", email);
    if (res.empty()) {
        return "";
    }
    return res[0]["password_hash"].as<std::string>();
}

void AuthManager::createUser(const std::string &email,
                             const std::string &password,
                             const std::string &name) {
    const auto res = db->execute(
        "INSERT INTO users (email, password_hash, name) VALUES ($1, $2, $3)",
        email, password, name);
}