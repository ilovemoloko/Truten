//
// Created by asavelev on 3/9/26.
//

#ifndef TRUTEN_SERVER_QUEUE_ROUTES_HPP
#define TRUTEN_SERVER_QUEUE_ROUTES_HPP

#include "crow.h"
#include "response_builder.hpp"

struct QueueRoutes {
public:
    void registerRoutes(crow::SimpleApp &app) {
        CROW_ROUTE(app, "/v1/queue/<string>/join")
                .methods("POST"_method)(
                    // TODO: not mvp
                    [](const crow::request &req, const std::string &slot_id) {
                        return ResponseBuilder().build();
                    });

        CROW_ROUTE(app, "/v1/queue/<string>/leave")
                .methods("POST"_method)(
                    [](const crow::request &req, const std::string &slot_id) {
                        // TODO: not mvp
                        return ResponseBuilder().build();
                    });
    }
};

#endif // TRUTEN_SERVER_QUEUE_ROUTES_HPP
