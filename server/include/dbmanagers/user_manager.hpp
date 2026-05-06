#ifndef TRUTEN_SERVER_USER_MANAGER_HPP
#define TRUTEN_SERVER_USER_MANAGER_HPP
#include "database.hpp"

// Responsible for user-related stuff.
// Includes but is not limited to hours, admin status, ban status
struct UserManager {
public:
    explicit UserManager(std::shared_ptr<Database> database)
        : db(std::move(database)) {
    };

    void addUserHours(const std::string &id, int amount);

    void banUser(const std::string &id, int duration);

    [[nodiscard]] bool isAdmin(const std::string &id) const;

    [[nodiscard]] bool userExists(const std::string &id) const;

    [[nodiscard]] int getUserHours(const std::string &id) const;

    [[nodiscard]] int getUnbanTime(const std::string &id) const;

    std::vector<std::string> getUserEnrollments(const std::string& user_id) const;

    std::string getNameById(const std::string& user_id) const;

    void addEnrollment(const std::string& user_id, const std::string& slot_id);

    void removeEnrollment(const std::string& user_id, const std::string& slot_id);

<<<<<<< Updated upstream
=======
    void addQueuedSlot(const std::string& user_id, const std::string& slot_id);

    void removeQueuedSlot(const std::string& user_id, const std::string& slot_id);

    [[nodiscard]] std::vector<std::string> getUserQueuedSlots(const std::string& user_id) const;

>>>>>>> Stashed changes
    void deleteUser(const std::string &id);

private:
    std::shared_ptr<Database> db;
};

#endif //TRUTEN_SERVER_USER_MANAGER_HPP