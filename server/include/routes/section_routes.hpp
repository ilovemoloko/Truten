#ifndef TRUTEN_SERVER_SECTION_ROUTES_HPP
#define TRUTEN_SERVER_SECTION_ROUTES_HPP

#include "crow.h"
#include "response_builder.hpp"
#include <string>
#include <utility>
#include <vector>
#include "slot_manager.hpp"
#include "user_manager.hpp"

struct SectionRoutes {
public:
    explicit SectionRoutes(const std::shared_ptr<Database> &db) : db_slot(db), db_user(db) {
    }

    [[nodiscard]] crow::response getGyms() const {
        auto gyms = db_slot.getGymList();
        crow::json::wvalue gym_list = std::move(gyms);
        crow::json::wvalue final;
        final["sections"] = std::move(gym_list);
        return {std::move(final)};
    }

    [[nodiscard]] crow::response getGymSlots(const std::string &gym_id) const {
        auto slots = db_slot.getGymSlots(gym_id);
        crow::json::wvalue slot_list = std::move(slots);
        crow::json::wvalue final;
        final["slots"] = std::move(slot_list);

        return {std::move(final)};
    }

    crow::response addGym(const crow::request& req) {
        RequestHandler request(req);
        request.require("gymName");
        request.require("creatorId");
        if (!request.responseIsOk()) {
            return ResponseBuilder(request).build();
        }
        auto gym_name = static_cast<std::string>(request["gymName"]);
        auto creator_id = static_cast<std::string>(request["creatorId"]);
        if (!db_user.isAdmin(creator_id)) {
            return ResponseBuilder(RESPONSE_CODE::NO_ACCESS).build();
        }
        db_slot.createGym(gym_name, creator_id);
        return ResponseBuilder().build();
    }

    crow::response addAdminToGym(const crow::request& req, const std::string& gym_id) {
        RequestHandler request(req);
        request.require("userId");
        if (!request.responseIsOk()) {
            return ResponseBuilder(request).build();
        }
        auto new_admin_id = static_cast<std::string>(request["userId"]);
        db_slot.addAdminToGym(gym_id, new_admin_id);
        return ResponseBuilder().build();
    }

    crow::response getAdminsByGym(const std::string& gym_id) const {
        const auto res = db_slot.getGymAdmins(gym_id);
        crow::json::wvalue wval = std::move(res);
        crow::json::wvalue final;
        final["admins"] = std::move(wval);
        return {std::move(final)};
    }

    void registerRoutes(crow::SimpleApp &app) {
        CROW_ROUTE(app, "/v1/sections/gymList").methods("GET"_method)(
            // TODO: add campus selection
            [this](const crow::request &req) {
                return getGyms();
            });

        CROW_ROUTE(app, "/v1/sections/gymList").methods("POST"_method)(
            [this](const crow::request &req) {
                return addGym(req);
            });

        CROW_ROUTE(app, "/v1/sections/<string>/admins").methods("POST"_method)(
            [this](const crow::request& req, const std::string& gym_id) {
                return addAdminToGym(req, gym_id);
            });

        CROW_ROUTE(app, "/v1/sections/<string>/admins").methods("GET"_method)(
            [this](const crow::request& req, const std::string& gym_id) {
                return getAdminsByGym(gym_id);
            });

        CROW_ROUTE(app, "/v1/sections/<string>/slots")(
            [this](const crow::request &req, const std::string &gym_id) {
                return getGymSlots(gym_id);
            });
    }

private:
    SlotManager db_slot;
    UserManager db_user;
};

#endif // TRUTEN_SERVER_SECTION_ROUTES_HPP
