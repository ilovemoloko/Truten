#ifndef TRUTEN_SERVER_USER_ROUTES_HPP
#define TRUTEN_SERVER_USER_ROUTES_HPP

#include "crow.h"
#include "database.hpp"
#include "request_handler.hpp"
#include "response_builder.hpp"
#include "status_codes.hpp"
#include "user_manager.hpp"

struct UserRoutes {
public:
    explicit UserRoutes(std::shared_ptr<Database> db);

    crow::response getUserHours(const std::string& user_id) const;
    crow::response addUserHours(const crow::request& req, const std::string& user_id);
    crow::response banUser(const crow::request& req, const std::string& user_id);
    crow::response getUnbanTime(const std::string& user_id) const;
    crow::response deleteUser(const std::string& id);
    crow::response getUserEnrollments(const std::string& id);
    crow::response getUserQueuedSlots(const std::string& id);

    template<typename AppType>
    void registerRoutes(AppType &app) {
        CROW_ROUTE(app, "/v1/user/<string>/gainedHours")(
            [this](const crow::request &req, const std::string &user_id) {
                return getUserHours(user_id);
            });
        CROW_ROUTE(app, "/v1/user/<string>/gainedHours")
                .methods("POST"_method)(
                    [this](const crow::request &req, const std::string &user_id) {
                        return addUserHours(req, user_id);
                    });

        CROW_ROUTE(app, "/v1/user/<string>/isBanned")
                .methods("POST"_method)(
                    [this](const crow::request &req, const std::string &user_id) {
                        return banUser(req, user_id);
                    });

        CROW_ROUTE(app, "/v1/user/<string>/unbanTime")(
            [this](const crow::request &req, const std::string &user_id) {
                return getUnbanTime(user_id);
            });

        CROW_ROUTE(app, "/v1/user/<string>/stats")(
            [this](const crow::request &req, const std::string &user_id) {
                // TODO: not mvp
                return ResponseBuilder().build();
            });

        CROW_ROUTE(app, "/v1/user/<string>/account")
                .methods("DELETE"_method)(
                    [this](const crow::request &req, const std::string &id) {
                        return deleteUser(id);
                    });

        CROW_ROUTE(app, "/v1/user/<string>/enrollments").methods("GET"_method)(
            [this](const crow::request& req, const std::string& user_id) {
                return getUserEnrollments(user_id);
            });

        CROW_ROUTE(app, "/v1/user/<string>/queuedSlots").methods("GET"_method)(
            [this](const crow::request& req, const std::string& user_id) {
                return getUserQueuedSlots(user_id);
            });
    }


private:
    std::shared_ptr<Database> db;
    UserManager db_user;
};

#endif // TRUTEN_SERVER_USER_ROUTES_HPP