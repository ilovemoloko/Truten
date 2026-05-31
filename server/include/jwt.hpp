#pragma once
#include <jwt-cpp/jwt.h>
#include <cstdlib>
#include <chrono>
#include "response_builder.hpp"
#include "status_codes.hpp"

static std::string jwt_secret() {
    const char *s = std::getenv("JWT_SECRET");
    return s ? std::string(s) : std::string("dev-secret");
}

static std::string make_access_token(const std::string &user_id, bool is_admin) {
    using namespace std::chrono;
    const auto now = system_clock::now();
    return jwt::create()
            .set_type("JWT")
            .set_issued_at(now)
            .set_expires_at(now + minutes(15))
            .set_payload_claim("sub", jwt::claim(user_id))
            .set_payload_claim("admin", jwt::claim(std::string(is_admin ? "1" : "0")))
            .sign(jwt::algorithm::hs256{jwt_secret()});
}

static std::string make_refresh_token(const std::string &user_id) {
    using namespace std::chrono;
    const auto now = system_clock::now();
    return jwt::create()
            .set_type("JWT")
            .set_issued_at(now)
            .set_expires_at(now + hours(24 * 7))
            .set_payload_claim("sub", jwt::claim(user_id))
            .set_payload_claim("typ", jwt::claim(std::string("refresh")))
            .sign(jwt::algorithm::hs256{jwt_secret()});
}

struct AuthMiddleware {
    struct context {
        std::string user_id;
        bool is_admin = false;
    };

    static bool starts_with(const std::string &s, const std::string &prefix) {
        return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
    }

    void before_handle(crow::request &req, crow::response &res, context &ctx) {
        const std::string path = req.url;

        if (starts_with(path, "/v1/auth/")) {
            return;
        }

        const std::string auth = req.get_header_value("Authorization");
        constexpr const char *kPrefix = "Bearer ";
        if (auth.rfind(kPrefix, 0) != 0 || auth.size() <= 7) {
            res.code = *RESPONSE_CODE::NO_ACCESS;
            res.set_header("Content-Type", "application/json");
            res.write(ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Missing Bearer token").build().body);
            res.end();
            return;
        }
        const std::string token = auth.substr(7);
        try {
            auto decoded = jwt::decode(token);
            jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{jwt_secret()})
                .verify(decoded);

            if (!decoded.has_payload_claim("sub")) {
                res.code = *RESPONSE_CODE::NO_ACCESS;
                res.set_header("Content-Type", "application/json");
                res.write(ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Token missing sub").build().body);
                res.end();
                return;
            }

            if (decoded.has_payload_claim("typ")) {
                try {
                    if (decoded.get_payload_claim("typ").as_string() == "refresh") {
                        res.code = *RESPONSE_CODE::NO_ACCESS;
                        res.set_header("Content-Type", "application/json");
                        res.write(ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Use access token").build().body);
                        res.end();
                        return;
                    }
                } catch (...) {}
            }

            ctx.user_id = decoded.get_payload_claim("sub").as_string();
            if (decoded.has_payload_claim("admin")) {
                try { ctx.is_admin = (decoded.get_payload_claim("admin").as_string() == "1"); } catch (...) {}
            }
        } catch (...) {
            res.code = *RESPONSE_CODE::NO_ACCESS;
            res.set_header("Content-Type", "application/json");
            res.write(ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Invalid token").build().body);
            res.end();
        }
    }

    void after_handle(crow::request &, crow::response &, context &) {}
};
