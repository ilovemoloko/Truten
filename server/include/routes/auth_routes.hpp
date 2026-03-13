//
// Created by asavelev on 3/9/26.
//

#ifndef TRUTEN_SERVER_AUTH_ROUTES_HPP
#define TRUTEN_SERVER_AUTH_ROUTES_HPP

#include "crow.h"
#include "database.hpp"
#include "request_handler.hpp"
#include "response_builder.hpp"

struct AuthRoutes {
public:
    using crow::json::type::String;

    explicit AuthRoutes(const Database &db) : db_auth(db) {
    }

    // I think the password should be hashed by the time this function is called
    void registerRoutes(crow::SimpleApp &app) {
        CROW_ROUTE(app, "/v1/auth/createAccount")
                .methods("POST"_method)([this](const crow::request &req) {
                    RequestHandler request(req);
                    request.require("email", String);
                    request.require("password", String);
                    request.require("name", String);
                    if (!request.responseIsOk()) {
                        return ResponseBuilder(request).build();
                    }
                    const auto email = static_cast<std::string>(request["email"]);
                    const auto password = static_cast<std::string>(request["password"]);
                    const auto name = static_cast<std::string>(request["name"]);
                    db_auth.createUser(email, password, name);
                    std::string userId = db_auth.getUserIdByEmail(email);
                    return ResponseBuilder(request).addField("userId", userId).build();
                });

        CROW_ROUTE(app, "/v1/auth/login")
                .methods("POST"_method)([this](const crow::request &req) {
                    RequestHandler request(req);
                    request.require("email", String);
                    request.require("password", String);
                    if (!request.responseIsOk()) {
                        return ResponseBuilder(request).build();
                    }
                    const auto email = static_cast<std::string>(request["email"]);
                    const auto password = static_cast<std::string>(request["password"]);
                    const std::string real_password = db_auth.getPasswordByEmail(email);
                    if (password == real_password) {
                        // TODO: not mvp, but add JWT
                        return ResponseBuilder(RESPONSE_CODE::OK_EMPTY).build();
                    }
                    return ResponseBuilder(RESPONSE_CODE::INVALID).build();
                });
        // TODO: not mvp, but add JWT
        CROW_ROUTE(app, "/v1/auth/refresh")
                .methods("POST"_method)(
                    [](const crow::request &req) { return ResponseBuilder().build(); });
    }

private:
    AuthManager db_auth;
};

#endif // TRUTEN_SERVER_AUTH_ROUTES_HPP
