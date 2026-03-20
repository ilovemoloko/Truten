#include "database.hpp"
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

