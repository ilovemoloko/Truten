#ifndef TRUTEN_SERVER_SECTION_ROUTES_HPP
#define TRUTEN_SERVER_SECTION_ROUTES_HPP

#include "crow.h"
#include "response_builder.hpp"
#include <string>
#include <vector>
#include "slot_manager.hpp"

struct SectionRoutes {
public:
    explicit SectionRoutes(std::shared_ptr<Database> db) : db_slot(std::move(db)) {}

    [[nodiscard]] crow::response getGyms() const {
        auto gyms = db_slot.getGymList();
        crow::json::wvalue final = std::move(gyms);
        return {std::move(final)};
    }

    void registerRoutes(crow::SimpleApp &app) {
        CROW_ROUTE(app, "/v1/sections/gymList")(
            // TODO: add campus selection
            [this](const crow::request &req) {
                return getGyms();
            });
    }

private:
    SlotManager db_slot;
};

#endif // TRUTEN_SERVER_SECTION_ROUTES_HPP
