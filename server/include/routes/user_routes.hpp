//
// Created by asavelev on 3/9/26.
//

#ifndef TRUTEN_SERVER_USER_ROUTES_HPP
#define TRUTEN_SERVER_USER_ROUTES_HPP

#include "crow.h"
#include "database.hpp"
#include "request_handler.hpp"
#include "response_builder.hpp"
#include "status_codes.hpp"

struct UserRoutes {
public:
    explicit UserRoutes(const Database &db) : db_user(db) {
    }

    void registerRoutes(crow::SimpleApp &app) {
        CROW_ROUTE(app, "/v1/user/<string>/gainedHours")(
            [this](const crow::request &req, const std::string &user_id) {
                if (!db_user.userExists(user_id)) {
                    return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
                }
                const int gained_hours = db_user.getUserHours(user_id);
                ResponseBuilder response;
                response.addField("gainedHours", gained_hours);
                return response.build();
            });
        CROW_ROUTE(app, "/v1/user/<string>/gainedHours")
                .methods("POST"_method)(
                    [this](const crow::request &req, const std::string &user_id) {
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
                    });

        CROW_ROUTE(app, "/v1/user/<string>/isBanned")
                .methods("POST"_method)(
                    [this](const crow::request &req, const std::string &user_id) {
                        if (!db_user.userExists(user_id)) {
                            return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
                        }
                        RequestHandler request(req);
                        request.require("banDuration", crow::json::type::Number);
                        const int duration = static_cast<int>(request["banDuration"]);
                        db_user.banUser(user_id, duration);
                        return ResponseBuilder(RESPONSE_CODE::OK_EMPTY).build();
                    });

        CROW_ROUTE(app, "/v1/user/<string>/unbanTime")(
            [this](const crow::request &req, const std::string &user_id) {
                if (!db_user.userExists(user_id)) {
                    return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
                }
                const int unban_time = db_user.getUnbanTime(user_id);
                return ResponseBuilder().addField("unbanTime", unban_time).build();
            });

        CROW_ROUTE(app, "/v1/user/<string>/stats")(
            [this](const crow::request &req, const std::string &user_id) {
                // TODO: not mvp
                return ResponseBuilder().build();
            });

        CROW_ROUTE(app, "/v1/user/<string>/account")
                .methods("DELETE"_method)(
                    [this](const crow::request &req, const std::string &id) {
                        db_user.deleteUser(id);
                        return ResponseBuilder(RESPONSE_CODE::OK_EMPTY).build();
                    });
    }

private:
    UserManager db_user;
};

#endif // TRUTEN_SERVER_USER_ROUTES_HPP
