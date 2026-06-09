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
    explicit AuthRoutes(const std::shared_ptr<Database>& db);

    crow::response createAccount(const crow::request &req);
    crow::response login(const crow::request &req) const;
    crow::response refresh(const crow::request &req) const;

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
    std::shared_ptr<Database> db_;
    AuthManager db_auth;
    UserManager db_user;
};

#endif // TRUTEN_SERVER_AUTH_ROUTES_HPP