#include "database.hpp"
#include <iostream>

thread_local DBTransaction* Database::current_txn = nullptr;

void Database::loadEnv(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR: .env file not found.\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (auto eqPos = line.find('='); eqPos != std::string::npos) {
            std::string key = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);
            setenv(key.c_str(), value.c_str(), 1);
        }
    }
}

std::shared_ptr<pqxx::connection> Database::acquireConnection() {
    std::unique_lock l(m_mutex);
    m_cond_var.wait(l, [this] { return !m_conn_pool.empty(); });
    auto available_conn = std::move(m_conn_pool.front());
    m_conn_pool.pop();
    return {available_conn.release(), [this](pqxx::connection* c) {
        std::unique_lock l1(m_mutex);
        m_conn_pool.push(std::unique_ptr<pqxx::connection>(c));
        m_cond_var.notify_one();
    }};
}

void Database::init() {
    executeInTransaction([this]() {
        execute("CREATE EXTENSION IF NOT EXISTS \"uuid-ossp\"");

#ifdef DROP_TABLES
        execute("DROP TABLE IF EXISTS users, slots, gyms");
#endif

        execute("CREATE TABLE IF NOT EXISTS users ("
                "ID UUID PRIMARY KEY DEFAULT uuid_generate_v4(),"
                "hours SMALLINT DEFAULT 0,"
                "email TEXT NOT NULL UNIQUE,"
                "name TEXT NOT NULL,"
                "password_hash TEXT NOT NULL,"
                "is_admin BOOLEAN DEFAULT FALSE,"
                "enrolled_slots UUID[],"
                "unban_time TIMESTAMP,"
                "queued_slots UUID[])");

        execute("CREATE TABLE IF NOT EXISTS slots ("
                "ID UUID PRIMARY KEY DEFAULT uuid_generate_v4(),"
                "section_id UUID DEFAULT uuid_generate_v4(),"
                "section_name TEXT NOT NULL,"
                "capacity SMALLINT DEFAULT 30,"
                "enrolled UUID[],"
                "start_time TIMESTAMP NOT NULL,"
                "end_time TIMESTAMP NOT NULL,"
                "is_cancelled BOOLEAN DEFAULT FALSE,"
                "queue UUID[])");

        execute("CREATE TABLE IF NOT EXISTS gyms ("
                "ID UUID PRIMARY KEY DEFAULT uuid_generate_v4(),"
                "name TEXT NOT NULL UNIQUE,"
                "admins UUID[])");

        execute("ALTER TABLE IF EXISTS users ADD COLUMN IF NOT EXISTS queued_slots UUID[]");
        execute("ALTER TABLE IF EXISTS slots ADD COLUMN IF NOT EXISTS queue UUID[]");

        // executeInTransaction() is template function. Return smth to avoid 'incomplete type'
        return 1;
    });
}
