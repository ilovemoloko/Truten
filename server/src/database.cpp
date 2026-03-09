//
// Created by asavelev on 2/20/26.
//
#include "../include/database.hpp"
#include "iostream"

void Database::init() {
    pqxx::work txn(conn);

    txn.exec("CREATE EXTENSION IF NOT EXISTS \"uuid-ossp\"");

#ifdef DROP_TABLES
    txn.exec("DROP TABLE users");
    txn.exec("DROP TABLE  slots");
    txn.exec("DROP TABLE gyms");
#endif

    txn.exec("CREATE TABLE IF NOT EXISTS users ("
        "ID UUID PRIMARY KEY DEFAULT uuid_generate_v4(),"
        "hours SMALLINT DEFAULT 0,"
        "email TEXT,"
        "name TEXT,"
        "password_hash TEXT,"
        "is_admin BOOLEAN DEFAULT FALSE,"
        "unban_time TIMESTAMP)");

    txn.exec("CREATE TABLE IF NOT EXISTS slots ("
        "ID UUID PRIMARY KEY DEFAULT uuid_generate_v4(),"
        "section_id UUID DEFAULT uuid_generate_v4(),"
        "section_name TEXT,"
        "capacity SMALLINT,"
        "enrolled UUID[],"
        "start_time TIMESTAMP,"
        "end_time TIMESTAMP,"
        "is_cancelled BOOLEAN DEFAULT FALSE)");

    txn.exec("CREATE TABLE IF NOT EXISTS gyms ("
        "ID UUID PRIMARY KEY DEFAULT uuid_generate_v4(),"
        "name TEXT,"
        "admins UUID[])");
    txn.commit();
}

pqxx::result SlotManager::getSlotInfo(const std::string &id) const {
    pqxx::work txn(conn);
    auto res = txn.exec_params("SELECT * FROM slots WHERE ID = $1", id);
    return res;
}

void SlotManager::changeSlotInfo(const std::string &id,
                                 const std::string &start_time,
                                 const std::string &end_time, int capacity) {
    pqxx::work txn(conn);
    txn.exec_params("UPDATE slots SET capacity = $1, start_time = $2, end_time = "
                    "$3 WHERE ID = $4",
                    capacity, start_time, end_time, id);
    txn.commit();
}

void SlotManager::removeEntry(const std::string &user_id,
                              const std::string &slot_id) {
    pqxx::work txn(conn);
    txn.exec_params(
        "UPDATE slots SET enrolled = array_remove(enrolled, $1) WHERE ID = $2",
        user_id, slot_id);
    txn.commit();
}

void SlotManager::addEntry(const std::string &user_id,
                           const std::string &slot_id) {
    pqxx::work txn(conn);
    txn.exec_params(
        "UPDATE slots SET enrolled = array_append(enrolled, $1) WHERE ID = $2",
        user_id, slot_id);
    txn.commit();
}

void SlotManager::closeSlot(const std::string &id) {
    pqxx::work txn(conn);
    txn.exec_params("UPDATE slots SET is_cancelled = TRUE WHERE ID = $1", id);
    txn.commit();
}

void UserManager::addUserHours(const std::string &id, int amount) {
    pqxx::work txn(conn);
    txn.exec_params("UPDATE users SET hours = hours + $1 WHERE ID = $2", amount,
                    id);
    txn.commit();
}

void UserManager::banUser(const std::string &id, int duration) {
    int new_time = static_cast<int>(time(nullptr)) + duration * 60 * 60;
    pqxx::work txn(conn);
    txn.exec_params("UPDATE users SET unban_time = unban_time + $1 WHERE ID = $2",
                    new_time, id);
    txn.commit();
}

bool UserManager::isAdmin(const std::string &id) const {
    pqxx::work txn(conn);
    const auto resp =
            txn.exec_params("SELECT is_admin FROM users WHERE ID = $1", id);
    return resp.begin()[0].as<bool>();
}

bool UserManager::userExists(const std::string &id) const {
    pqxx::work txn(conn);
    const auto resp = txn.exec_params("SELECT * FROM users WHERE ID = $1", id);
    return !resp.empty();
}

int UserManager::getUnbanTime(const std::string &id) const {
    pqxx::work txn(conn);
    const auto resp =
            txn.exec_params("SELECT unbanTime FROM users WHERE ID = $1", id);
    return resp.begin()[0].as<int>();
}

int UserManager::getUserHours(const std::string &id) const {
    pqxx::work txn(conn);
    const auto resp =
            txn.exec_params("SELECT hours FROM users WHERE ID = $1", id);
    return resp.begin()[0].as<int>();
}

bool AuthManager::emailExists(const std::string &email) const {
    pqxx::work txn(conn);
    const auto res =
            txn.exec_params("SELECT 1 FROM users WHERE email = $1", email);
    return !res.empty();
}

std::string AuthManager::getUserIdByEmail(const std::string &email) const {
    pqxx::work txn(conn);
    const auto res =
            txn.exec_params("SELECT id FROM users WHERE email = $1", email);
    if (res.empty()) {
        return "";
    }
    return res[0]["id"].as<std::string>();
}

std::string AuthManager::getPasswordByEmail(const std::string &email) const {
    pqxx::work txn(conn);
    const auto res = txn.exec_params(
        "SELECT password_hash FROM users WHERE email = $1", email);
    if (res.empty()) {
        return "";
    }
    return res[0]["password_hash"].as<std::string>();
}

void AuthManager::createUser(const std::string &email,
                             const std::string &password,
                             const std::string &name) {
    pqxx::work txn(conn);
    const auto res = txn.exec_params(
        "INSERT INTO users (email, password_hash, name) VALUES ($1, $2, $3)",
        email, password, name);
    txn.commit();
}

void UserManager::deleteUser(const std::string &id) {
    pqxx::work txn(conn);
    const auto res = txn.exec_params("DELETE FROM users WHERE id = $1", id);
}
