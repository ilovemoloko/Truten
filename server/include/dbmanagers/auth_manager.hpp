#ifndef TRUTEN_SERVER_AUTH_MANAGER_HPP
#define TRUTEN_SERVER_AUTH_MANAGER_HPP

#include "database.hpp"
#include <string>


// Essentially everything associated with auth
// Includes but is not limited to emails, passwords, JWT(?), IDs
struct AuthManager {
public:
    explicit AuthManager(std::shared_ptr<Database> database) : db(std::move(database)) {

    }

    [[nodiscard]] std::string getUserIdEmail(const std::string &email) const;

    [[nodiscard]] std::string getUserIdByEmail(const std::string &email) const;

    [[nodiscard]] std::string getPasswordByEmail(const std::string &email) const;

    [[nodiscard]] bool emailExists(const std::string &email) const;

    void createUser(const std::string &email, const std::string &password,
                    const std::string &name, bool is_admin = false);
private:
    std::shared_ptr<Database> db;
};

#endif //TRUTEN_SERVER_AUTH_MANAGER_HPP