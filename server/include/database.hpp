#ifndef SERVER_DATABASE_HPP
#define SERVER_DATABASE_HPP

#include <fstream>
#include <mutex>
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

    template<typename ...Args>
    auto execute(const std::string& command, Args&&... args) {
        std::unique_lock l(mutex);
        pqxx::work txn(conn);
        auto res = txn.exec_params(command, std::forward<Args>(args)...);
        txn.commit();
        return res;
    }
    static void loadEnv(const std::string& path = ".env");

private:
    const std::string conn_string;
    mutable std::mutex mutex;
    mutable pqxx::connection conn;
};

#endif // SERVER_DATABASE_HPP
