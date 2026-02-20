//
// Created by asavelev on 2/17/26.
//
#include "crow.h"
#include "status_codes.hpp"
#include "request_handler.hpp"
#include "response_builder.hpp"
#include "database.hpp"

int main() {
    const std::string connection_string = "dbname=testdb " //TODO: learn how to do this properly
            "user=postgres "
            "host=localhost "
            "port=5432";

    Database db(connection_string);
    db.init();
    crow::SimpleApp app;

    CROW_ROUTE(app, "/v1/user/<string>/gainedHours")([&db](const crow::request &req, const std::string &user_id) {
        bool found_user = db.userExists(user_id);
        if (!found_user) {
            return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
        }
        const int gained_hours = db.getUserHours(user_id);
        ResponseBuilder response;
        response.addField("gainedHours", gained_hours);
        return response.build();
    });
    CROW_ROUTE(app, "/v1/user/<string>/gainedHours").methods("POST"_method)(
        [&db](const crow::request &req, const std::string &user_id) {
            requestHandler request(req);
            request.require("hours", crow::json::type::Number);
            if (!db.addUserHours(user_id)) {
                return ResponseBuilder(RESPONSE_CODE::BAD_GATEWAY).build();
            }
            return ResponseBuilder(request).build();
        });

    CROW_ROUTE(app, "/v1/user/<string>/isBanned").methods("POST"_method)(
        [&db](const crow::request &req, const std::string &user_id) {
            bool foundUser = db.userExists(user_id);
            if (!foundUser) {
                return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
            }
            requestHandler request(req);
            request.require("banDuration", crow::json::type::Number);
            int duration = static_cast<int>(request["banDuration"]);
            if (!db.isAdmin(user_id)) {
                return ResponseBuilder(RESPONSE_CODE::NO_ACCESS).build();
            }
            if (!db.banUser(user_id, duration)) {
                return ResponseBuilder(RESPONSE_CODE::BAD_GATEWAY).build();
            }
            return ResponseBuilder(RESPONSE_CODE::OK_EMPTY).build();
        });


    CROW_ROUTE(app, "/v1/user/<string>/unbanTime")(
        [&db](const crow::request &req, const std::string &user_id) {
            bool foundUser = db.userExists(user_id);
            if (!foundUser) {
                return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
            }
            const int unban_time = db.getUnbanTime(user_id);
            return ResponseBuilder().addField("unbanTime", unban_time).build();
        });

    CROW_ROUTE(app, "/v1/user/<string>/stats")(
        [&db](const crow::request &req, const std::string &user_id) {
            //TODO: add stats. Not for MVP.
            return ResponseBuilder().build();
        });
    //TODO: add error handling below
    CROW_ROUTE(app, "/v1/sections/gymList")(
        //TODO: add campus selection
        //TODO: gotta think how to implement this properly
        [&db](const crow::request &req) {
            return ResponseBuilder().build();
        });

    CROW_ROUTE(app, "/v1/slots/<string>").methods("GET"_method)(
        [&db](const crow::request &req, const std::string &slot_id) {
            ResponseBuilder response;
            auto res = db.getSlotInfo(slot_id)[0];
            //abomination
            //TODO: think of something better than this
            response.addField("slot_id", res["id"].as<std::string>());
            response.addField("section_id", res["section_id"].as<std::string>());
            response.addField("section_name", res["section_name"].as<std::string>());
            response.addField("capacity", res["capacity"].as<int>());
            response.addField("enrolled", res["enrolled"].as<std::string>());
            response.addField("start_time", res["start_time"].as<std::string>());
            response.addField("end_time", res["end_time"].as<std::string>());
            response.addField("is_cancelled", res["is_cancelled"].as<bool>());
            return response.build();
        });

    CROW_ROUTE(app, "/v1/slots/<string>").methods("PATCH"_method)(
        [&db](const crow::request &req, const std::string &slot_id) {
            requestHandler request(req);
            request.require("startTime", crow::json::type::String);
            request.require("endTime", crow::json::type::String);
            request.require("capacity", crow::json::type::Number);
            if (request.responseIsOk()) {
                const auto start_time = std::string(request["startTime"]);
                const auto end_time = std::string(request["endTime"]);
                const int capacity = static_cast<int>(request["capacity"]);
                db.changeSlotInfo(slot_id, start_time, end_time, capacity);
            }
            return ResponseBuilder(request).build();
        });

    CROW_ROUTE(app, "/v1/slots/<string>").methods("DELETE"_method)(
        [&db](const crow::request &req, const std::string &slot_id) {
            db.closeSlot(slot_id);
            return ResponseBuilder().build();
        });

    CROW_ROUTE(app, "/v1/slots/<string>/entries").methods("POST"_method)(
        [&db](const crow::request &req, const std::string &slot_id) {
            requestHandler request(req);
            request.require("userId", crow::json::type::String);
            if (request.responseIsOk()) {
                const auto user_id = std::string(request["userId"]);
                db.addEntry(user_id, slot_id);
            }
            return ResponseBuilder(request).build();
        });

    CROW_ROUTE(app, "/v1/slots/<string>/entries").methods("DELETE"_method)(
        [&db](const crow::request &req, const std::string &slot_id) {
            requestHandler request(req);
            request.require("userId", crow::json::type::String);
            if (request.responseIsOk()) {
                const auto user_id = std::string(request["userId"]);
                db.removeEntry(user_id, slot_id);
            }
            return ResponseBuilder(request).build();
        });

    CROW_ROUTE(app, "/v1/queue/<string>/join").methods("POST"_method)(
        //TODO: not mvp
        [](const crow::request &req, const std::string &slot_id) {
            return ResponseBuilder().build();
        });

    CROW_ROUTE(app, "/v1/queue/<string>/leave").methods("POST"_method)(
        [](const crow::request &req, const std::string &slot_id) {
            //TODO: not mvp
            return ResponseBuilder().build();
        });

    CROW_ROUTE(app, "/v1/auth/createAccount").methods("POST"_method)(
        [](const crow::request &req) {
            return ResponseBuilder().build();
        });

    CROW_ROUTE(app, "/v1/auth/login").methods("POST"_method)(
        [](const crow::request &req) {
            return ResponseBuilder().build();
        });

    CROW_ROUTE(app, "/v1/auth/refresh").methods("POST"_method)(
        [](const crow::request &req) {
            return ResponseBuilder().build();
        });

    CROW_ROUTE(app, "/v1/user/account").methods("DELETE"_method)(
        [](const crow::request &req) {
            return ResponseBuilder().build();
        });

    app.port(8080).multithreaded().run();

    return 0;
}
