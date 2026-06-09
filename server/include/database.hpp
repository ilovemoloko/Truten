#ifndef SERVER_DATABASE_HPP
#define SERVER_DATABASE_HPP

#include <fstream>
#include <mutex>
#include <pqxx/pqxx>
#include <string>
#include <queue>
#include <condition_variable>
#include <memory>
#include "transaction_guard.hpp"

struct Database {
public:
    virtual ~Database() {
        std::unique_lock l(m_mutex);
        while (!m_conn_pool.empty()) {
            m_conn_pool.pop();
        }
    }
    explicit Database(const std::string &connect_string)
        : m_conn_string(connect_string) {
        for (int i = 0; i < 12; i++) {
            m_conn_pool.push(std::make_unique<pqxx::connection>(connect_string));
        }
    }

    void init();

    [[nodiscard]] std::string getConnectionString() const { return m_conn_string; }

    std::shared_ptr<pqxx::connection> acquireConnection();

    template<typename ...Args>
    auto execute(const std::string& command, Args&&... args);
    
    template<typename Func>
    auto executeInTransaction(Func&& f);

    static void loadEnv(const std::string& path = ".env");
    static void printEnvStatus();

    static thread_local DBTransaction* current_txn;
private:
    const std::string m_conn_string;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond_var;
    std::queue<std::unique_ptr<pqxx::connection>> m_conn_pool;
};

// use for simple single transactions
template<typename ...Args>
auto Database::execute(const std::string& command, Args&&... args) {
    TransactionGuard guard(*this);
    auto res = current_txn->execute(command, std::forward<Args>(args)...);
    guard.commit();
    return res;
}

// use for multiple operations in one transaction without commits for each operation
template<typename Func>
auto Database::executeInTransaction(Func&& f) {
    TransactionGuard txn(*this);
    auto res = f();
    txn.commit();
    return res;
}

#endif // SERVER_DATABASE_HPP
