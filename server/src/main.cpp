//
// Created by asavelev on 2/17/26.
//
#include "crow.h"
#include "status_codes.hpp"
#include "request_handler.hpp"
#include "response_builder.hpp"
#include "database.hpp"

#include <jwt-cpp/jwt.h>
#include <cstdlib>
#include <chrono>

static std::string jwt_secret() {
    const char* s = std::getenv("JWT_SECRET");
    if (s) {
        return std::string(s);
    } else {
        return std::string("dev-secret");
    }
}//return secret for jwt 

static std::string make_access_token(const std::string& user_id, bool is_admin) {
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto exp = now + minutes(15);
    return jwt::create()
        .set_type("JWT")
        .set_issued_at(now)
        .set_expires_at(exp)
        .set_payload_claim("sub", jwt::claim(user_id))
        .set_payload_claim("admin", jwt::claim(is_admin))
        .sign(jwt::algorithm::hs256{jwt_secret()});
}//making short-life token

static std::string make_refresh_token(const std::string& user_id) {
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto exp = now + hours(24 * 7); // 7 days

    return jwt::create()
        .set_type("JWT")
        .set_issued_at(now)
        .set_expires_at(exp)
        .set_payload_claim("sub", jwt::claim(user_id))
        .set_payload_claim("typ", jwt::claim(std::string("refresh")))
        .sign(jwt::algorithm::hs256{jwt_secret()});
}//making like long-life token(maybe should be smaller)

struct AuthMiddleware {
    struct context {
        std::string user_id;
        bool is_admin{false};
    };

    static bool starts_with(const std::string& s, const std::string& prefix) {
        return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
    }

    void before_handle(crow::request& req, crow::response& res, context& ctx) {
        const std::string path = req.url;

        if (starts_with(path, "/v1/auth/")) {
            return;
        }//do not check because have not got token yet

        const std::string auth = req.get_header_value("Authorization");
        constexpr const char* kPrefix = "Bearer ";
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
            jwt::verify().allow_algorithm(jwt::algorithm::hs256{jwt_secret()}).verify(decoded);//check our token
            if (!decoded.has_payload_claim("sub")) {
                res.code = *RESPONSE_CODE::NO_ACCESS;
                res.set_header("Content-Type", "application/json");
                res.write(ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Token missing sub").build().body);
                res.end();
                return;
            }

            ctx.user_id = decoded.get_payload_claim("sub").as_string();
            if (decoded.has_payload_claim("admin")) {
                try { ctx.is_admin = decoded.get_payload_claim("admin").as_bool(); } catch (...) {}
            }
        } catch (...) {
            res.code = *RESPONSE_CODE::NO_ACCESS;
            res.set_header("Content-Type", "application/json");
            res.write(ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Invalid token").build().body);
            res.end();
        }
    }

    void after_handle(crow::request&, crow::response&, context&) {
        //maybe will be some realisation if we need do smth after
    }
};
int main() {
    const std::string connection_string = "dbname=testdb " //TODO: learn how to do this properly
            "user=postgres "
            "host=localhost "
            "port=5432";

    Database db(connection_string);
    db.init();
    crow::SimpleApp app;

    CROW_ROUTE(app, "/v1/user/<string>/gainedHours")([&db](const crow::request &req, const std::string &user_id) {
        bool found_user = db.userExists(user_id);
        if (!found_user) {
            return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
        }
        const int gained_hours = db.getUserHours(user_id);
        ResponseBuilder response;
        response.addField("gainedHours", gained_hours);
        return response.build();
    });
    CROW_ROUTE(app, "/v1/user/<string>/gainedHours").methods("POST"_method)(
        [&db](const crow::request &req, const std::string &user_id) {
            bool found_user = db.userExists(user_id);
            if (!found_user) {
                return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
            }
            requestHandler request(req);
            request.require("hours", crow::json::type::Number);
            db.addUserHours(user_id);
            return ResponseBuilder(request).build();
        });

    CROW_ROUTE(app, "/v1/user/<string>/isBanned").methods("POST"_method)(
        [&db](const crow::request &req, const std::string &user_id) {
            bool foundUser = db.userExists(user_id);
            if (!foundUser) {
                return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
            }
            requestHandler request(req);
            request.require("banDuration", crow::json::type::Number);
            int duration = static_cast<int>(request["banDuration"]);
            if (!db.isAdmin(user_id)) {
                return ResponseBuilder(RESPONSE_CODE::NO_ACCESS).build();
            }
            db.banUser(user_id, duration);
            return ResponseBuilder(RESPONSE_CODE::OK_EMPTY).build();
        });


    CROW_ROUTE(app, "/v1/user/<string>/unbanTime")(
        [&db](const crow::request &req, const std::string &user_id) {
            bool foundUser = db.userExists(user_id);
            if (!foundUser) {
                return ResponseBuilder(RESPONSE_CODE::NOT_FOUND).build();
            }
            const int unban_time = db.getUnbanTime(user_id);
            return ResponseBuilder().addField("unbanTime", unban_time).build();
        });

    CROW_ROUTE(app, "/v1/user/<string>/stats")(
        [&db](const crow::request &req, const std::string &user_id) {
            //TODO: not mvp
            return ResponseBuilder().build();
        });
    CROW_ROUTE(app, "/v1/sections/gymList")(
        //TODO: add campus selection
        //TODO: gotta think how to implement this properly
        [&db](const crow::request &req) {
            return ResponseBuilder().build();
        });

    CROW_ROUTE(app, "/v1/slots/<string>").methods("GET"_method)(
        [&db](const crow::request &req, const std::string &slot_id) {
            ResponseBuilder response;
            auto res = db.getSlotInfo(slot_id)[0];
            for (std::string field : {"slot_id", "section_id", "section_name", "enrolled", "start_time", "end_time"}) {
                response.addField(field, res[field].as<std::string>());
            }
            response.addField("capacity", res["capacity"].as<int>());
            response.addField("is_cancelled", res["is_cancelled"].as<bool>());
            return response.build();
        });

    CROW_ROUTE(app, "/v1/slots/<string>").methods("PATCH"_method)(
        [&db](const crow::request &req, const std::string &slot_id) {
            requestHandler request(req);
            request.require("startTime", crow::json::type::String);
            request.require("endTime", crow::json::type::String);
            request.require("capacity", crow::json::type::Number);
            if (request.responseIsOk()) {
                const auto start_time = std::string(request["startTime"]);
                const auto end_time = std::string(request["endTime"]);
                const int capacity = static_cast<int>(request["capacity"]);
                db.changeSlotInfo(slot_id, start_time, end_time, capacity);
            }
            return ResponseBuilder(request).build();
        });

    CROW_ROUTE(app, "/v1/slots/<string>").methods("DELETE"_method)(
        [&db](const crow::request &req, const std::string &slot_id) {
            db.closeSlot(slot_id);
            return ResponseBuilder(RESPONSE_CODE::OK_EMPTY).build();
        });

    CROW_ROUTE(app, "/v1/slots/<string>/entries").methods("POST"_method)(
        [&db](const crow::request &req, const std::string &slot_id) {
            requestHandler request(req);
            request.require("userId", crow::json::type::String);
            if (request.responseIsOk()) {
                const auto user_id = std::string(request["userId"]);
                db.addEntry(user_id, slot_id);
            }
            return ResponseBuilder(request).build();
        });

    CROW_ROUTE(app, "/v1/slots/<string>/entries").methods("DELETE"_method)(
        [&db](const crow::request &req, const std::string &slot_id) {
            requestHandler request(req);
            request.require("userId", crow::json::type::String);
            if (request.responseIsOk()) {
                const auto user_id = std::string(request["userId"]);
                db.removeEntry(user_id, slot_id);
            }
            return ResponseBuilder(request).build();
        });

    CROW_ROUTE(app, "/v1/queue/<string>/join").methods("POST"_method)(
        //TODO: not mvp
        [](const crow::request &req, const std::string &slot_id) {
            return ResponseBuilder().build();
        });

    CROW_ROUTE(app, "/v1/queue/<string>/leave").methods("POST"_method)(
        [](const crow::request &req, const std::string &slot_id) {
            //TODO: not mvp
            return ResponseBuilder().build();
        });
    //TODO: would be in few days maybe
    CROW_ROUTE(app, "/v1/auth/createAccount").methods("POST"_method)(
        [](const crow::request &req) {
            return ResponseBuilder().build();
        });

    CROW_ROUTE(app, "/v1/auth/login").methods("POST"_method)(
        [](const crow::request &req) {
            return ResponseBuilder().build();
        });

    CROW_ROUTE(app, "/v1/auth/refresh").methods("POST"_method)(
        [](const crow::request &req) {
            return ResponseBuilder().build();
        });

    CROW_ROUTE(app, "/v1/user/account").methods("DELETE"_method)(
        [](const crow::request &req) {
            return ResponseBuilder().build();
        });

    app.port(8080).multithreaded().run();

    return 0;
}
