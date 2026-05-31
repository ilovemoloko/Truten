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
    explicit QueueRoutes(const std::shared_ptr<Database>& db) : db_(db), db_slots(db), db_user(db) {}

    crow::response joinQueue(const crow::request& req, const std::string& slot_id) {
        RequestHandler request(req);
        request.require("userId", crow::json::type::String);
        if (!request.responseIsOk()) {
            return ResponseBuilder(request).build();
        }
        const auto user_id = std::string(request["userId"]);

        return db_->executeInTransaction([&]() {
            const auto enrollments = db_user.getUserEnrollments(user_id, true);
            if (std::find(enrollments.begin(), enrollments.end(), slot_id) != enrollments.end()) {
                return ResponseBuilder(RESPONSE_CODE::INVALID, "Already enrolled").build();
            }

            const auto queued = db_user.getUserQueuedSlots(user_id, true);
            if (std::find(queued.begin(), queued.end(), slot_id) != queued.end()) {
                return ResponseBuilder(RESPONSE_CODE::INVALID, "Already in queue").build();
            }

            if (!db_slots.isAtCapacity(slot_id, true)) {
                db_slots.addEntry(user_id, slot_id);
                db_user.addEnrollment(user_id, slot_id);
                ResponseBuilder resp;
                resp.addField("enrolled", true);
                return resp.build();
            }

            db_slots.addToQueue(user_id, slot_id);
            db_user.addQueuedSlot(user_id, slot_id);
            ResponseBuilder resp;
            resp.addField("enrolled", false);
            resp.addField("queued", true);
            return resp.build();
        });
    }

    crow::response leaveQueue(const crow::request& req, const std::string& slot_id) {
        RequestHandler request(req);
        request.require("userId", crow::json::type::String);
        if (!request.responseIsOk()) {
            return ResponseBuilder(request).build();
        }
        const auto user_id = std::string(request["userId"]);

        return db_->executeInTransaction([&]() {
            const auto queued = db_user.getUserQueuedSlots(user_id, true);
            if (std::find(queued.begin(), queued.end(), slot_id) == queued.end()) {
                return ResponseBuilder(RESPONSE_CODE::NOT_FOUND, "Not in queue").build();
            }

            db_slots.removeFromQueue(user_id, slot_id);
            db_user.removeQueuedSlot(user_id, slot_id);
            return ResponseBuilder(RESPONSE_CODE::OK_EMPTY).build();
        });
    }

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
