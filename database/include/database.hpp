//
// Created by asavelev on 2/17/26.
//

#ifndef SERVER_DATABASE_HPP
#define SERVER_DATABASE_HPP

#include <pqxx/pqxx>
#include <string>
#include <memory>
#include <optional>

class Database {
public:
    Database();
    ~Database();

    bool connect(const std::string& conn_str);
    void disconnect();

    bool createTable();

    bool createUser(const std::string& email, const std::string& password);
    bool deleteUser(std::string id);
    bool addUserHours(std::string id);
    bool banUser(std::string id, int duration);
    bool isAdmin(std::string id);

    int getUserHours(std::string id);

    bool isConnected() const { return conn != nullptr && conn->is_open(); }

private:
    std::unique_ptr<pqxx::connection> conn;
    std::string connection_string;
};

#endif //SERVER_DATABASE_HPP