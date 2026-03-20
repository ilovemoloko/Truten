#ifndef TRUTEN_SERVER_SECTION_ROUTES_HPP
#define TRUTEN_SERVER_SECTION_ROUTES_HPP

#include "crow.h"
#include "response_builder.hpp"

struct SectionRoutes {
public:
    void registerRoutes(crow::SimpleApp &app) {
        CROW_ROUTE(app, "/v1/sections/gymList")(
            // TODO: add campus selection
            // TODO: gotta think how to implement this properly
            [](const crow::request &req) { return ResponseBuilder().build(); });
    }
};

#endif // TRUTEN_SERVER_SECTION_ROUTES_HPP
