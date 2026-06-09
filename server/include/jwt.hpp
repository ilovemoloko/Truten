#pragma once
#include <jwt-cpp/jwt.h>
#include <string>
#include "crow.h"
#include "response_builder.hpp"
#include "status_codes.hpp"

std::string jwt_secret();
std::string make_access_token(const std::string &user_id, bool is_admin);
std::string make_refresh_token(const std::string &user_id);

struct AuthMiddleware {
    struct context {
        std::string user_id;
        bool is_admin = false;
    };

    static bool starts_with(const std::string &s, const std::string &prefix);
    void before_handle(crow::request &req, crow::response &res, context &ctx);
    void after_handle(crow::request &, crow::response &, context &);
};