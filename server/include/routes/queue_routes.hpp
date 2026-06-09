#ifndef TRUTEN_SERVER_QUEUE_ROUTES_HPP
#define TRUTEN_SERVER_QUEUE_ROUTES_HPP

#include "crow.h"
#include "database.hpp"
#include "request_handler.hpp"
#include "response_builder.hpp"
#include "status_codes.hpp"
#include "slot_manager.hpp"
#include "user_manager.hpp"

struct QueueRoutes {
public:
    explicit QueueRoutes(const std::shared_ptr<Database>& db);

    crow::response joinQueue(const crow::request& req, const std::string& slot_id);
    crow::response leaveQueue(const crow::request& req, const std::string& slot_id);

    template<typename AppType>
    void registerRoutes(AppType &app) {
        CROW_ROUTE(app, "/v1/queue/<string>/join")
                .methods("POST"_method)(
                    [this](const crow::request &req, const std::string &slot_id) {
                        return joinQueue(req, slot_id);
                    });

        CROW_ROUTE(app, "/v1/queue/<string>/leave")
                .methods("POST"_method)(
                    [this](const crow::request &req, const std::string &slot_id) {
                        return leaveQueue(req, slot_id);
                    });
    }

private:
    std::shared_ptr<Database> db_;
    SlotManager db_slots;
    UserManager db_user;
};

#endif // TRUTEN_SERVER_QUEUE_ROUTES_HPP