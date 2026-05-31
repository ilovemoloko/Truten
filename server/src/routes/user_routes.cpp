#include "user_routes.hpp"

UserRoutes::UserRoutes(std::shared_ptr<Database> db) : db(db), db_user(std::move(db)) {
}

crow::response UserRoutes::getUserHours(const std::string& user_id) const {
    if (!db_user.userExists(user_id)) {
        return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
    }
    const int gained_hours = db_user.getUserHours(user_id);
    ResponseBuilder response;
    response.addField("gainedHours", gained_hours);
    return response.build();
}

crow::response UserRoutes::addUserHours(const crow::request& req, const std::string& user_id) {
    RequestHandler request(req);
    request.require("hours", crow::json::type::Number);
    if (!request.responseIsOk()) {
        return ResponseBuilder(request).build();
    }
    const int hours = static_cast<int>(request["hours"]);

    return db->executeInTransaction([&]() {
        if (!db_user.userExists(user_id, true)) {
            return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
        }
        db_user.addUserHours(user_id, hours);
        return ResponseBuilder(request).build();
    });
}

crow::response UserRoutes::banUser(const crow::request& req, const std::string& user_id) {
    RequestHandler request(req);
    request.require("banDuration", crow::json::type::Number);
    if (!request.responseIsOk()) {
        return ResponseBuilder(request).build();
    }
    const int duration = static_cast<int>(request["banDuration"]);

    return db->executeInTransaction([&]() {
        if (!db_user.userExists(user_id, true)) {
            return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
        }
        db_user.banUser(user_id, duration);
        return ResponseBuilder(RESPONSE_CODE::OK_EMPTY).build();
    });
}

crow::response UserRoutes::getUnbanTime(const std::string& user_id) const {
    if (!db_user.userExists(user_id)) {
        return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
    }
    const int unban_time = db_user.getUnbanTime(user_id);
    return ResponseBuilder().addField("unbanTime", unban_time).build();
}

crow::response UserRoutes::deleteUser(const std::string& id) {
    return db->executeInTransaction([&]() {
        db_user.deleteUser(id);
        return ResponseBuilder(RESPONSE_CODE::OK_EMPTY).build();
    });
}

crow::response UserRoutes::getUserEnrollments(const std::string& id) {
    const auto res = db_user.getUserEnrollments(id);
    ResponseBuilder response;
    response.addField("enrollments", res);
    return response.build();
}

crow::response UserRoutes::getUserQueuedSlots(const std::string& id) {
    const auto res = db_user.getUserQueuedSlots(id);
    ResponseBuilder response;
    response.addField("queuedSlots", res);
    return response.build();
}