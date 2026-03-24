#ifndef TRUTEN_SERVER_SLOT_ROUTES_HPP
#define TRUTEN_SERVER_SLOT_ROUTES_HPP

#include "crow.h"
#include "database.hpp"
#include "request_handler.hpp"
#include "response_builder.hpp"
#include "status_codes.hpp"
#include "slot_manager.hpp"

struct SlotRoutes {
public:
    explicit SlotRoutes(std::shared_ptr<Database> db) : db_slots(std::move(db)) {
    }

    crow::response getSlots(const std::string& slot_id) const {
        ResponseBuilder response;
        auto res = db_slots.getSlotInfo(slot_id)[0];
        for (std::string field : {
                 "slot_id", "section_id", "section_name",
                 "enrolled", "start_time", "end_time"
             }) {
            response.addField(field, res[field].as<std::string>());
             }
        response.addField("capacity", res["capacity"].as<int>());
        response.addField("is_cancelled", res["is_cancelled"].as<bool>());
        return response.build();
    }

    crow::response patchSlot(const crow::request& req, const std::string& slot_id) {
        RequestHandler request(req);
        request.require("startTime", crow::json::type::String);
        request.require("endTime", crow::json::type::String);
        request.require("capacity", crow::json::type::Number);
        if (request.responseIsOk()) {
            const auto start_time = std::string(request["startTime"]);
            const auto end_time = std::string(request["endTime"]);
            const int capacity = static_cast<int>(request["capacity"]);
            db_slots.changeSlotInfo(slot_id, start_time, end_time, capacity);
        }
        return ResponseBuilder(request).build();
    }

    crow::response deleteSlot(const std::string& slot_id) {
        db_slots.closeSlot(slot_id);
        return ResponseBuilder(RESPONSE_CODE::OK_EMPTY).build();
    }

    crow::response addUserEntry(const crow::request& req, const std::string& slot_id) {
        RequestHandler request(req);
        request.require("userId", crow::json::type::String);
        if (request.responseIsOk()) {
            const auto user_id = std::string(request["userId"]);
            db_slots.addEntry(user_id, slot_id);
        }
        return ResponseBuilder(request).build();
    }

    crow::response deleteUserEntry(const crow::request& req, const std::string& slot_id) {
        RequestHandler request(req);
        request.require("userId", crow::json::type::String);
        if (request.responseIsOk()) {
            const auto user_id = std::string(request["userId"]);
            db_slots.removeEntry(user_id, slot_id);
        }
        return ResponseBuilder(request).build();
    }

    crow::response getGymList() const {
        ResponseBuilder res;
        auto [gyms, ids] = db_slots.getGymList();
        res.addField("sections", gyms);
        res.addField("ids", ids);
        return res.build();
    }

    void registerRoutes(crow::SimpleApp &app) {
        CROW_ROUTE(app, "/v1/slots/<string>")
                .methods("GET"_method)(
                    [this](const crow::request &req, const std::string &slot_id) {
                        return getSlots(slot_id);
                    });

        CROW_ROUTE(app, "/v1/slots/<string>")
                .methods("PATCH"_method)([this](const crow::request &req,
                                                const std::string &slot_id) {
                    return patchSlot(req, slot_id);
                });

        CROW_ROUTE(app, "/v1/slots/<string>")
                .methods("DELETE"_method)(
                    [this](const crow::request &req, const std::string &slot_id) {
                        return deleteSlot(slot_id);
                    });

        CROW_ROUTE(app, "/v1/slots/<string>/entries")
                .methods("POST"_method)(
                    [this](const crow::request &req, const std::string &slot_id) {
                        return addUserEntry(req, slot_id);
                    });

        CROW_ROUTE(app, "/v1/slots/<string>/entries")
                .methods("DELETE"_method)(
                    [this](const crow::request &req, const std::string &slot_id) {
                        return deleteUserEntry(req, slot_id);
                    });

        CROW_ROUTE(app, "v1/sections/gymList").methods("GET"_method)(
            [this](const crow::request& req) {
                return getGymList();
            });
    }

private:
    SlotManager db_slots;
};

#endif // TRUTEN_SERVER_SLOT_ROUTES_HPP
