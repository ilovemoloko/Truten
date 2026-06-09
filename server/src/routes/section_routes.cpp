#include "section_routes.hpp"

SectionRoutes::SectionRoutes(const std::shared_ptr<Database> &db) : db_(db), db_slot(db), db_user(db) {
}

crow::response SectionRoutes::getGyms() const {
    auto gyms = db_slot.getGymList();
    crow::json::wvalue gym_list = std::move(gyms);
    crow::json::wvalue final;
    final["sections"] = std::move(gym_list);
    return {std::move(final)};
}

crow::response SectionRoutes::getGymSlots(const std::string &gym_id) const {
    auto slots = db_slot.getGymSlots(gym_id);
    crow::json::wvalue slot_list = std::move(slots);
    crow::json::wvalue final;
    final["slots"] = std::move(slot_list);
    return {std::move(final)};
}

crow::response SectionRoutes::addGym(const crow::request& req) {
    RequestHandler request(req);
    request.require("gymName");
    request.require("creatorId");
    if (!request.responseIsOk()) {
        return ResponseBuilder(request).build();
    }
    auto gym_name = static_cast<std::string>(request["gymName"]);
    auto creator_id = static_cast<std::string>(request["creatorId"]);
    return db_->executeInTransaction([&]() {
        if (!db_user.isAdmin(creator_id, true)) {
            return ResponseBuilder(RESPONSE_CODE::NO_ACCESS).build();
        }
        db_slot.createGym(gym_name, creator_id);
        return ResponseBuilder().build();
    });
}

crow::response SectionRoutes::addAdminToGym(const crow::request& req, const std::string& gym_id) {
    RequestHandler request(req);
    request.require("userId");
    if (!request.responseIsOk()) {
        return ResponseBuilder(request).build();
    }
    auto new_admin_id = static_cast<std::string>(request["userId"]);
    db_slot.addAdminToGym(gym_id, new_admin_id);
    return ResponseBuilder().build();
}

crow::response SectionRoutes::getAdminsByGym(const std::string& gym_id) const {
    const auto res = db_slot.getGymAdmins(gym_id);
    crow::json::wvalue wval = std::move(res);
    crow::json::wvalue final;
    final["admins"] = std::move(wval);
    return {std::move(final)};
}