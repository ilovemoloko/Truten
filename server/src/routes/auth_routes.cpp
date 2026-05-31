#include "auth_routes.hpp"

AuthRoutes::AuthRoutes(const std::shared_ptr<Database>& db) : db_(db), db_auth(db), db_user(db) {
}

crow::response AuthRoutes::createAccount(const crow::request &req) {
    RequestHandler request(req);
    request.require("email", crow::json::type::String);
    request.require("password", crow::json::type::String);
    request.require("name", crow::json::type::String);
    if (!request.responseIsOk()) {
        return ResponseBuilder(request).build();
    }
    const auto email = static_cast<std::string>(request["email"]);
    const auto password = static_cast<std::string>(request["password"]);
    const auto name = static_cast<std::string>(request["name"]);
    
    return db_->executeInTransaction([&]() {
        if (db_auth.emailExists(email, true)) {
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
    });
}

crow::response AuthRoutes::login(const crow::request &req) const {
    RequestHandler request(req);
    request.require("email", crow::json::type::String);
    request.require("password", crow::json::type::String);
    if (!request.responseIsOk()) {
        return ResponseBuilder(request).build();
    }
    const auto email = static_cast<std::string>(request["email"]);
    const auto password = static_cast<std::string>(request["password"]);
    
    return db_->executeInTransaction([&]() {
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
    });
}

crow::response AuthRoutes::refresh(const crow::request &req) const {
    RequestHandler request(req);
    request.require("refreshToken", crow::json::type::String);
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
        return db_->executeInTransaction([&]() {
            const bool is_admin = db_user.isAdmin(user_id);
            const std::string new_access = make_access_token(user_id, is_admin);
            ResponseBuilder resp;
            resp.addField("accessToken", new_access);
            return resp.build();
        });
    } catch (...) {
        return ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Invalid refresh token").build();
    }
}