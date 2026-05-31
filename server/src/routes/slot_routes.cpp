#include "slot_routes.hpp"
#include <algorithm>

SlotRoutes::SlotRoutes(const std::shared_ptr<Database>& db) : db(db), db_slots(db), db_user(db) {
}

crow::response SlotRoutes::slotInfo(const std::string& slot_id) const {
    auto res = db_slots.getSlotInfoJSON(slot_id);
    return {res};
}

crow::response SlotRoutes::createSlot(const crow::request& req) {
    RequestHandler request(req);
    request.require("startTime");
    request.require("endTime");
    request.require("gymId");
    request.require("capacity");
    if (!request.responseIsOk()) {
        return ResponseBuilder(request).build();
    }
    auto start = static_cast<std::string>(request["startTime"]);
    auto end = static_cast<std::string>(request["endTime"]);
    auto gym_id = static_cast<std::string>(request["gymId"]);
    auto capacity = static_cast<int>(request["capacity"]);
    return db->executeInTransaction([&]() {
        db_slots.createSlot(gym_id, start, end, capacity);
        return ResponseBuilder().build();
    });
}

crow::response SlotRoutes::patchSlot(const crow::request& req, const std::string& slot_id) {
    RequestHandler request(req);
    request.require("startTime", crow::json::type::String);
    request.require("endTime", crow::json::type::String);
    request.require("capacity", crow::json::type::Number);
    if (request.responseIsOk()) {
        const auto start_time = std::string(request["startTime"]);
        const auto end_time = std::string(request["endTime"]);
        const int capacity = static_cast<int>(request["capacity"]);
        return db->executeInTransaction([&]() {
            db_slots.changeSlotInfo(slot_id, start_time, end_time, capacity);
            return ResponseBuilder(request).build();
        });
    }
    return ResponseBuilder(request).build();
}

crow::response SlotRoutes::deleteSlot(const std::string& slot_id) {
    return db->executeInTransaction([&]() {
        db_slots.deleteSlot(slot_id);
        return ResponseBuilder(RESPONSE_CODE::OK_EMPTY).build();
    });
}

crow::response SlotRoutes::addUserEntry(const crow::request& req, const std::string& slot_id) {
    RequestHandler request(req);
    request.require("userId", crow::json::type::String);
    if (!request.responseIsOk()) {
        return ResponseBuilder(request).build();
    }
    const auto user_id = std::string(request["userId"]);

    return db->executeInTransaction([&]() {
        const auto user_enrollments = db_user.getUserEnrollments(user_id, true);
        if (std::find(user_enrollments.begin(), user_enrollments.end(), slot_id) != user_enrollments.end()) {
            return ResponseBuilder(RESPONSE_CODE::INVALID, "Already enrolled").build();
        }
        if (db_slots.isAtCapacity(slot_id, true)) {
            return ResponseBuilder(RESPONSE_CODE::INVALID, "Slot is full. Join the queue instead.").build();
        }
        db_slots.addEntry(user_id, slot_id);
        db_user.addEnrollment(user_id, slot_id);
        return ResponseBuilder().build();
    });
}

crow::response SlotRoutes::deleteUserEntry(const crow::request& req, const std::string& slot_id) {
    RequestHandler request(req);
    request.require("userId", crow::json::type::String);
    if (!request.responseIsOk()) {
        return ResponseBuilder(request).build();
    }
    const auto user_id = std::string(request["userId"]);

    return db->executeInTransaction([&]() {
        db_slots.removeEntry(user_id, slot_id);
        db_user.removeEnrollment(user_id, slot_id);

        const std::string next = db_slots.getFirstInQueue(slot_id, true);
        if (!next.empty()) {
            db_slots.removeFromQueue(next, slot_id);
            db_user.removeQueuedSlot(next, slot_id);
            db_slots.addEntry(next, slot_id);
            db_user.addEnrollment(next, slot_id);
        }

        return ResponseBuilder(RESPONSE_CODE::OK_EMPTY).build();
    });
}