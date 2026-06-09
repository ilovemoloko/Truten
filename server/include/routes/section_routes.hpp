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
    explicit SectionRoutes(const std::shared_ptr<Database> &db);

    [[nodiscard]] crow::response getGyms() const;
    [[nodiscard]] crow::response getGymSlots(const std::string &gym_id) const;
    crow::response addGym(const crow::request& req);
    crow::response addAdminToGym(const crow::request& req, const std::string& gym_id);
    crow::response getAdminsByGym(const std::string& gym_id) const;

    template<typename AppType>
    void registerRoutes(AppType &app) {
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
    std::shared_ptr<Database> db_;
    SlotManager db_slot;
    UserManager db_user;
};

#endif // TRUTEN_SERVER_SECTION_ROUTES_HPP