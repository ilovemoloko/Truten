#include "queue_routes.hpp"
#include <algorithm>

QueueRoutes::QueueRoutes(const std::shared_ptr<Database>& db) : db_(db), db_slots(db), db_user(db) {}

crow::response QueueRoutes::joinQueue(const crow::request& req, const std::string& slot_id) {
    RequestHandler request(req);
    request.require("userId", crow::json::type::String);
    if (!request.responseIsOk()) {
        return ResponseBuilder(request).build();
    }
    const auto user_id = std::string(request["userId"]);

    return db_->executeInTransaction([&]() {
        // block rows in order slot, user (to avoid deadlock)
        db_slots.lockSlot(slot_id);
        const bool is_full = db_slots.isAtCapacity(slot_id);

        db_user.lockUser(user_id);
        const auto enrollments = db_user.getUserEnrollments(user_id);
        if (std::find(enrollments.begin(), enrollments.end(), slot_id) != enrollments.end()) {
            return ResponseBuilder(RESPONSE_CODE::INVALID, "Already enrolled").build();
        }

        const auto queued = db_user.getUserQueuedSlots(user_id);
        if (std::find(queued.begin(), queued.end(), slot_id) != queued.end()) {
            return ResponseBuilder(RESPONSE_CODE::INVALID, "Already in queue").build();
        }

        if (!is_full) {
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

crow::response QueueRoutes::leaveQueue(const crow::request& req, const std::string& slot_id) {
    RequestHandler request(req);
    request.require("userId", crow::json::type::String);
    if (!request.responseIsOk()) {
        return ResponseBuilder(request).build();
    }
    const auto user_id = std::string(request["userId"]);

    return db_->executeInTransaction([&]() {
        db_slots.lockSlot(slot_id);
        db_user.lockUser(user_id);

        const auto queued = db_user.getUserQueuedSlots(user_id);
        if (std::find(queued.begin(), queued.end(), slot_id) == queued.end()) {
            return ResponseBuilder(RESPONSE_CODE::NOT_FOUND, "Not in queue").build();
        }

        db_slots.removeFromQueue(user_id, slot_id);
        db_user.removeQueuedSlot(user_id, slot_id);
        return ResponseBuilder(RESPONSE_CODE::OK_EMPTY).build();
    });
}