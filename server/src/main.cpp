//
// Created by asavelev on 2/17/26.
//
#include "crow.h"
#include "status_codes.hpp"
#include "request_handler.hpp"
#include "response_builder.hpp"
#include "database.hpp"

const std::string connection_string;

int main() {
    Database db;
    db.connect(connection_string);
    crow::SimpleApp app;

    // GET /v1/user/123/gainedHours
    CROW_ROUTE(app, "/v1/user/<string>/gainedHours")([&db](const crow::request &req, const std::string& user_id) {
        const int gained_hours = db.getUserHours(user_id);
        if (gained_hours == -1) {
            return responseBuilder(RESPONSE_CODE::NOT_FOUND).build();
        }
        responseBuilder response;
        response.addField("gainedHours", gained_hours);
        return response.build();
    });
    CROW_ROUTE(app, "/v1/user/<string>/gainedHours").methods("POST"_method)(
        [&db](const crow::request &req, const std::string& user_id) {
            requestHandler request(req);
            request.require("hours", crow::json::type::Number);
            if (!db.addUserHours(user_id)) {
                return responseBuilder(RESPONSE_CODE::BAD_GATEWAY).build();
            }
            return responseBuilder(request.firstResponse()).build();
        });

    CROW_ROUTE(app, "v1/user/<string>/isBanned").methods("POST"_method)(
        [&db](const crow::request &req, const std::string& user_id) {
            requestHandler request(req);
            request.require("banDuration", crow::json::type::Number);
            int duration = static_cast<int>(request["banDuration"]);
            if (!db.isAdmin(user_id)) {
                return responseBuilder(RESPONSE_CODE::NO_ACCESS).build();
            }
            if (!db.banUser(user_id, duration)) {
                return responseBuilder(RESPONSE_CODE::BAD_GATEWAY).build();
            }
            return responseBuilder(RESPONSE_CODE::OK_EMPTY).build();
        });
    app.port(8080).multithreaded().run();

    return 0;
}
