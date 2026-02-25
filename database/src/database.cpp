//
// Created by asavelev on 2/20/26.
//
#include "database.hpp"
#include "iostream"
#include "status_codes.hpp"

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
        "email TEXT,"
        "name TEXT,"
        "password_hash TEXT,"
        "is_admin BOOLEAN DEFAULT FALSE,"
        "unban_time TIMESTAMP)"
    );

    txn.exec("CREATE TABLE IF NOT EXISTS slots ("
        "ID UUID PRIMARY KEY DEFAULT uuid_generate_v4(),"
        "section_id UUID DEFAULT uuid_generate_v4(),"
        "section_name TEXT,"
        "capacity SMALLINT,"
        "enrolled UUID[],"
        "start_time TIMESTAMP,"
        "end_time TIMESTAMP,"
        "is_cancelled BOOLEAN DEFAULT FALSE)"
    );

    txn.exec("CREATE TABLE IF NOT EXISTS gyms ("
        "ID UUID PRIMARY KEY DEFAULT uuid_generate_v4(),"
        "name TEXT,"
        "admins UUID[])");
    txn.commit();
}

pqxx::result Database::getSlotInfo(const std::string &id) {
    pqxx::work txn(conn);
    auto res = txn.exec_params("SELECT * FROM slots WHERE ID = $1", id);
    return res;
}

void Database::changeSlotInfo(const std::string &id, const std::string &start_time, const std::string &end_time,
                              int capacity) {
    pqxx::work txn(conn);
    txn.exec_params("UPDATE slots SET capacity = $1, start_time = $2, end_time = $3 WHERE ID = $4", capacity,
                    start_time, end_time, id);
    txn.commit();
}

void Database::removeEntry(const std::string &user_id, const std::string &slot_id) {
    pqxx::work txn(conn);
    txn.exec_params("UPDATE slots SET enrolled = array_remove(enrolled, $1) WHERE ID = $2", user_id, slot_id);
    txn.commit();
}

void Database::addEntry(const std::string &user_id, const std::string &slot_id) {
    pqxx::work txn(conn);
    txn.exec_params("UPDATE slots SET enrolled = array_append(enrolled, $1) WHERE ID = $2", user_id, slot_id);
    txn.commit();
}

void Database::closeSlot(const std::string &id) {
    pqxx::work txn(conn);
    txn.exec_params("UPDATE slots SET is_cancelled = TRUE WHERE ID = $1", id);
    txn.commit();
}

void Database::addUserHours(const std::string &id, int amount) {
    pqxx::work txn(conn);
    txn.exec_params("UPDATE users SET hours = hours + 2 WHERE ID = $1", id);
    txn.commit();
}

void Database::banUser(const std::string &id, int duration) {
    int new_time = static_cast<int>(time(nullptr)) + duration * 60 * 60;
    pqxx::work txn(conn);
    txn.exec_params("UPDATE users SET unban_time = unban_time + $1 WHERE ID = $2", new_time, id);
    txn.commit();
}

bool Database::isAdmin(const std::string &id) {
    pqxx::work txn(conn);
    const auto resp = txn.exec_params("SELECT is_admin FROM users WHERE ID = $1", id);
    return resp.begin()[0].as<bool>();
}

bool Database::userExists(const std::string &id) {
    pqxx::work txn(conn);
    const auto resp = txn.exec_params("SELECT * FROM users WHERE ID = $1", id);
    return !resp.empty();
}

int Database::getUnbanTime(const std::string &id) {
    pqxx::work txn(conn);
    const auto resp = txn.exec_params("SELECT unbanTime FROM users WHERE ID = $1", id);
    return resp.begin()[0].as<int>();
}

int Database::getUserHours(const std::string &id) {
    pqxx::work txn(conn);
    const auto resp = txn.exec_params("SELECT hours FROM users WHERE ID = $1", id);
    return resp.begin()[0].as<int>();
}
