#ifndef SERVER_TRANSACTION_GUARD_HPP
#define SERVER_TRANSACTION_GUARD_HPP

#include <pqxx/pqxx>
#include <memory>

struct Database;

class DBTransaction {
public:
    explicit DBTransaction(std::shared_ptr<pqxx::connection> conn)
        : m_conn(std::move(conn)), m_txn(std::make_unique<pqxx::work>(*m_conn)) {
    }

    ~DBTransaction() {
        if (!m_finished && m_txn) {
            // dctr must be noexcept
            try { m_txn->abort(); } catch(...) {}
        }
    }

    template<typename ...Args>
    auto execute(const std::string& command, Args&&... args) {
        return m_txn->exec_params(command, std::forward<Args>(args)...);
    }

    void commit() {
        m_txn->commit();
        m_finished = true;
    }

    void rollback() {
        m_txn->abort();
        m_finished = true;
    }

private:
    std::shared_ptr<pqxx::connection> m_conn;
    std::unique_ptr<pqxx::work> m_txn;
    bool m_finished = false;
};

class TransactionGuard {
public:
    explicit TransactionGuard(Database& db);
    ~TransactionGuard();
    void commit();
    void rollback();
private:
    std::unique_ptr<DBTransaction> m_owned_txn;
};

#endif // SERVER_TRANSACTION_GUARD_HPP
