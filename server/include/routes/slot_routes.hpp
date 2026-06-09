#ifndef TRUTEN_SERVER_SLOT_ROUTES_HPP
#define TRUTEN_SERVER_SLOT_ROUTES_HPP

#include "crow.h"
#include "database.hpp"
#include "request_handler.hpp"
#include "response_builder.hpp"
#include "status_codes.hpp"
#include "slot_manager.hpp"
#include "user_manager.hpp"

struct SlotRoutes {
public:
    explicit SlotRoutes(const std::shared_ptr<Database>& db);

    crow::response slotInfo(const std::string& slot_id) const;
    crow::response createSlot(const crow::request& req);
    crow::response patchSlot(const crow::request& req, const std::string& slot_id);
    crow::response deleteSlot(const std::string& slot_id);
    crow::response addUserEntry(const crow::request& req, const std::string& slot_id);
    crow::response deleteUserEntry(const crow::request& req, const std::string& slot_id);

    template<typename AppType>
    void registerRoutes(AppType &app) {
        CROW_ROUTE(app, "/v1/slots/<string>")
                .methods("GET"_method)(
                    [this](const crow::request &req, const std::string &slot_id) {
                        return slotInfo(slot_id);
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

        CROW_ROUTE(app, "/v1/slots")
        .methods("POST"_method)(
            [this](const crow::request &req) {
                return createSlot(req);
            });
    }

private:
    std::shared_ptr<Database> db;
    SlotManager db_slots;
    UserManager db_user;
};

#endif // TRUTEN_SERVER_SLOT_ROUTES_HPP