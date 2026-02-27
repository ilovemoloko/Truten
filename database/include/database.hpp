//
// Created by asavelev on 2/17/26.
//

#ifndef SERVER_DATABASE_HPP
#define SERVER_DATABASE_HPP

#include <pqxx/pqxx>
#include <string>
#include <iostream>

struct Database {
public:
    Database();

    Database(const std::string& connect_string) : conn(pqxx::connection(connect_string)) {
    }

    void execute(std::string& command) {
        pqxx::work txn(conn);
        txn.exec(command);
        txn.commit();
    }

    void init();
    //new auth methods
    bool emailExusts(const std::string& email);
    std::string getUserIdEmail(const std::string& email);
    std::string getPasswordByEmail(const std::string& email);
    //new auth methods
    bool createUser(const std::string& email, const std::string& password);//i think create and delete should be bool to get true, when we create succesfully
    bool deleteUser(std::string id);
    void addUserHours(const std::string &id, int amount = 2);
    void banUser(const std::string& id, int duration);
    bool isAdmin(const std::string& id);
    bool userExists(const std::string& id);

    void changeSlotInfo(const std::string& id, const std::string& start_time, const std::string& end_time, int capacity);
    void closeSlot(const std::string& id);

    void removeEntry(const std::string& user_id, const std::string& slot_id);
    void addEntry(const std::string& user_id, const std::string& slot_id);


    pqxx::result getSlotInfo(const std::string& id);

    std::vector<std::string> getGymList();

    [[nodiscard]] int getUserHours(const std::string& id);
    [[nodiscard]] int getUnbanTime(const std::string& id);

    [[nodiscard]] bool isConnected() const { return conn.is_open(); }

private:
    pqxx::connection conn;
};

#endif //SERVER_DATABASE_HPP
