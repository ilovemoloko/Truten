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
    explicit UserRoutes(std::shared_ptr<Database> db) : db_user(std::move(db)) {
    }

    crow::response getUserHours(const std::string& user_id) const {
        if (!db_user.userExists(user_id)) {
            return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
        }
        const int gained_hours = db_user.getUserHours(user_id);
        ResponseBuilder response;
        response.addField("gainedHours", gained_hours);
        return response.build();
    }

    crow::response addUserHours(const crow::request& req, const std::string& user_id) {
        if (!db_user.userExists(user_id)) {
            return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
        }
        RequestHandler request(req);
        request.require("hours", crow::json::type::Number);
        if (!request.responseIsOk()) {
            return ResponseBuilder(request).build();
        }
        const int hours = static_cast<int>(request["hours"]);
        db_user.addUserHours(user_id, hours);
        return ResponseBuilder(request).build();
    }

    crow::response banUser(const crow::request& req, const std::string& user_id) {
        if (!db_user.userExists(user_id)) {
            return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
        }
        RequestHandler request(req);
        request.require("banDuration", crow::json::type::Number);
        const int duration = static_cast<int>(request["banDuration"]);
        db_user.banUser(user_id, duration);
        return ResponseBuilder(RESPONSE_CODE::OK_EMPTY).build();
    }

    crow::response getUnbanTime(const std::string& user_id) const {
        if (!db_user.userExists(user_id)) {
            return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
        }
        const int unban_time = db_user.getUnbanTime(user_id);
        return ResponseBuilder().addField("unbanTime", unban_time).build();
    }

    crow::response deleteUser(const std::string& id) {
        db_user.deleteUser(id);
        return ResponseBuilder(RESPONSE_CODE::OK_EMPTY).build();
    }

    crow::response getUserEnrollments(const std::string& id) {
        const auto res = db_user.getUserEnrollments(id);
        ResponseBuilder response;
        response.addField("enrollments", res);
        return response.build();
    }

    void registerRoutes(crow::SimpleApp &app) {
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
    }

private:
    UserManager db_user;
};

#endif // TRUTEN_SERVER_USER_ROUTES_HPP
