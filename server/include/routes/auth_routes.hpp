#ifndef TRUTEN_SERVER_AUTH_ROUTES_HPP
#define TRUTEN_SERVER_AUTH_ROUTES_HPP

#include "crow.h"
#include "database.hpp"
#include "request_handler.hpp"
#include "response_builder.hpp"
#include "auth_manager.hpp"
#include "user_manager.hpp"
#include "jwt.hpp"

struct AuthRoutes {
public:
    using crow::json::type::String;

    explicit AuthRoutes(const std::shared_ptr<Database>& db) : db_auth(db), db_user(db) {
    }

    crow::response createAccount(const crow::request &req) {
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
        if (db_auth.emailExists(email)) {
            return ResponseBuilder(RESPONSE_CODE::INVALID).build();
        }
        db_auth.createUser(email, password, name);
        const std::string user_id = db_auth.getUserIdByEmail(email);
        const std::string access_token = make_access_token(user_id, false);
        const std::string refresh_token = make_refresh_token(user_id);
        ResponseBuilder resp;
        resp.addField("userId", user_id);
        resp.addField("isAdmin", false);
        resp.addField("accessToken", access_token);
        resp.addField("refreshToken", refresh_token);
        return resp.build();
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
        if (password != real_password) {
            return ResponseBuilder(RESPONSE_CODE::NO_ACCESS).build();
        }
        const bool is_admin = db_user.isAdmin(user_id);
        const std::string access_token = make_access_token(user_id, is_admin);
        const std::string refresh_token = make_refresh_token(user_id);
        ResponseBuilder resp;
        resp.addField("userId", user_id);
        resp.addField("isAdmin", is_admin);
        resp.addField("accessToken", access_token);
        resp.addField("refreshToken", refresh_token);
        return resp.build();
    }

    crow::response refresh(const crow::request &req) const {
        RequestHandler request(req);
        request.require("refreshToken", String);
        if (!request.responseIsOk()) {
            return ResponseBuilder(request).build();
        }
        const auto token_str = static_cast<std::string>(request["refreshToken"]);
        try {
            auto decoded = jwt::decode(token_str);
            jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{jwt_secret()})
                .verify(decoded);

            if (!decoded.has_payload_claim("typ") ||
                decoded.get_payload_claim("typ").as_string() != "refresh") {
                return ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Not a refresh token").build();
            }

            const std::string user_id = decoded.get_payload_claim("sub").as_string();
            const bool is_admin = db_user.isAdmin(user_id);
            const std::string new_access = make_access_token(user_id, is_admin);
            ResponseBuilder resp;
            resp.addField("accessToken", new_access);
            return resp.build();
        } catch (...) {
            return ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Invalid refresh token").build();
        }
    }

    template<typename AppType>
    void registerRoutes(AppType &app) {
        CROW_ROUTE(app, "/v1/auth/createAccount")
                .methods("POST"_method)([this](const crow::request &req) {
                    return createAccount(req);
                });

        CROW_ROUTE(app, "/v1/auth/login")
                .methods("POST"_method)([this](const crow::request &req) {
                    return login(req);
                });

        CROW_ROUTE(app, "/v1/auth/refresh")
                .methods("POST"_method)([this](const crow::request &req) {
                    return refresh(req);
                });
    }

private:
    AuthManager db_auth;
    UserManager db_user;
};

#endif // TRUTEN_SERVER_AUTH_ROUTES_HPP
