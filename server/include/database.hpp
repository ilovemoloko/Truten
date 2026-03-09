//
// Created by asavelev on 2/17/26.
//

#ifndef SERVER_DATABASE_HPP
#define SERVER_DATABASE_HPP

#include <pqxx/pqxx>
#include <string>

/*
 *CONST CORRECTNESS NOTE
 *Some methods are marked const, some are not,
 *despite essentially being the same thing:
 *just execute a single SQL prompt.
 *Despite them formally not changing
 *the object, I decided to only mark
 *methods that don't even affect data in the
 *database as const. Otherwise, it might be misleading.
 */

// Base class. It can do nothing.
struct Database {
public:
    virtual ~Database() = default;

    Database() = default;

    explicit Database(const std::string &connect_string)
        : conn_string(connect_string), conn(pqxx::connection(connect_string)) {
    }

    explicit Database(Database &&other) noexcept
        : conn_string(other.get_conn_string()), conn(std::move(other.conn)) {
    }

    void init();

    [[nodiscard]] std::string get_conn_string() const { return conn_string; }

    [[nodiscard]] bool isConnected() const { return conn.is_open(); }

protected:
    const std::string conn_string;
    mutable pqxx::connection conn;

    void execute(std::string command) const {
        pqxx::work txn(conn);
        txn.exec(std::move(command));
        txn.commit();
    }
};

// Responsible for user-related stuff.
// Includes but is not limited to hours, admin status, ban status
struct UserManager : Database {
    explicit UserManager(std::string connection_string)
        : Database(std::move(connection_string)) {
    };

    explicit UserManager(const Database &db)
        : UserManager(db.get_conn_string()) {
    }

    void addUserHours(const std::string &id, int amount = 2);

    void banUser(const std::string &id, int duration);

    bool isAdmin(const std::string &id) const;

    [[nodiscard]] bool userExists(const std::string &id) const;

    [[nodiscard]] int getUserHours(const std::string &id) const;

    [[nodiscard]] int getUnbanTime(const std::string &id) const;

    void deleteUser(const std::string &id);
};

// Essentially everything associated with auth
// Includes but is not limited to emails, passwords, JWT(?), IDs
struct AuthManager : Database {
    explicit AuthManager(std::string conn_string)
        : Database(std::move(conn_string)) {
    }

    explicit AuthManager(const Database &db)
        : AuthManager(db.get_conn_string()) {
    }

    [[nodiscard]] std::string getUserIdEmail(const std::string &email) const;

    [[nodiscard]] std::string getUserIdByEmail(const std::string &email) const;

    [[nodiscard]] std::string getPasswordByEmail(const std::string &email) const;

    [[nodiscard]] bool emailExists(const std::string &email) const;

    void createUser(const std::string &email, const std::string &password,
                    const std::string &name);
};

// I suppose you can guess what it does by the name. Come on. We're not
// 5-year-olds here. Go on.
struct SlotManager : Database {
    explicit SlotManager(std::string conn_string)
        : Database(std::move(conn_string)) {
    }

    explicit SlotManager(const Database &db)
        : SlotManager(db.get_conn_string()) {
    }

    void changeSlotInfo(const std::string &id, const std::string &start_time,
                        const std::string &end_time, int capacity);

    void closeSlot(const std::string &id);

    void removeEntry(const std::string &user_id, const std::string &slot_id);

    void addEntry(const std::string &user_id, const std::string &slot_id);

    pqxx::result getSlotInfo(const std::string &id) const;

    std::vector<std::string> getGymList() const; // TODO
};

#endif // SERVER_DATABASE_HPP
