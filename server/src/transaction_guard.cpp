#include "transaction_guard.hpp"
#include "database.hpp"

TransactionGuard::TransactionGuard(Database& db) {
    if (!Database::current_txn) {
        m_owned_txn = std::make_unique<DBTransaction>(db.acquireConnection());
        Database::current_txn = m_owned_txn.get();
    }
}

TransactionGuard::~TransactionGuard() {
    if (m_owned_txn) {
        Database::current_txn = nullptr;
    }
}

void TransactionGuard::commit() {
    if (m_owned_txn) {
        m_owned_txn->commit();
    }
}

void TransactionGuard::rollback() {
    if (m_owned_txn) {
        m_owned_txn->rollback();
    }
}
