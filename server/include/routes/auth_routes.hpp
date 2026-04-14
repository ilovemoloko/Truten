#ifndef TRUTEN_SERVER_AUTH_ROUTES_HPP
#define TRUTEN_SERVER_AUTH_ROUTES_HPP

#include "crow.h"
#include "database.hpp"
#include "request_handler.hpp"
#include "response_builder.hpp"
#include "auth_manager.hpp"

struct AuthRoutes {
public:
    using crow::json::type::String;

    explicit AuthRoutes(std::shared_ptr<Database> db) : db_auth(std::move(db)) {
    }

    crow::response createAccount(const crow::request &req) {
        RequestHandler request(req);
        request.require("email", String);
        request.require("password", String);
        request.require("name", String);
        request.require("isAdmin");
        if (!request.responseIsOk()) {
            return ResponseBuilder(request).build();
        }
        const auto email = static_cast<std::string>(request["email"]);
        const auto password = static_cast<std::string>(request["password"]);
        const auto name = static_cast<std::string>(request["name"]);
        const bool is_admin = request["isAdmin"].b();
        if (db_auth.emailExists(email)) {
            return ResponseBuilder(RESPONSE_CODE::INVALID).build();
        }
        db_auth.createUser(email, password, name, is_admin);
        const std::string user_id = db_auth.getUserIdByEmail(email);
        return ResponseBuilder(RESPONSE_CODE::OK).addField("userId", user_id).build();
    }

    crow::response login(const crow::request &req) const {
        RequestHandler request(req);
        request.require("email", String);
        request.require("password", String);
        if (!request.responseIsOk()) {
            return ResponseBuilder(request).build();
        }
        const auto email = static_cast<std::string>(request["email"]);
        const auto password = static_cast<std::string>(request["password"]);
        const std::string real_password = db_auth.getPasswordByEmail(email);
        const std::string user_id = db_auth.getUserIdByEmail(email);
        if (password == real_password) {
            // TODO: not mvp, but add JWT
            return ResponseBuilder(RESPONSE_CODE::OK).addField("userId", user_id).build();
        }
        return ResponseBuilder(RESPONSE_CODE::NO_ACCESS).build();
    }

    void registerRoutes(crow::SimpleApp &app) {
        CROW_ROUTE(app, "/v1/auth/createAccount")
                .methods("POST"_method)([this](const crow::request &req) {
                    return createAccount(req);
                });

        CROW_ROUTE(app, "/v1/auth/login")
                .methods("POST"_method)([this](const crow::request &req) {
                    return login(req);
                });
        // TODO: not mvp, but add JWT
        CROW_ROUTE(app, "/v1/auth/refresh")
                .methods("POST"_method)(
                    [](const crow::request &req) { return ResponseBuilder().build(); }
                    );
    }

private:
    AuthManager db_auth;
};

#endif // TRUTEN_SERVER_AUTH_ROUTES_HPP
