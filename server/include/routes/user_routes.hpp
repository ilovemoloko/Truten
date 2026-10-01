#ifndef TRUTEN_SERVER_USER_ROUTES_HPP
#define TRUTEN_SERVER_USER_ROUTES_HPP

#include "crow.h"
#include "database.hpp"
#include "jwt.hpp"
#include "request_handler.hpp"
#include "response_builder.hpp"
#include "status_codes.hpp"
#include "user_manager.hpp"

struct UserRoutes {
public:
    explicit UserRoutes(std::shared_ptr<Database> db);

    crow::response getUserHours(const std::string& user_id) const;
    crow::response addUserHours(const crow::request& req, const std::string& user_id);
    crow::response banUser(const crow::request& req, const std::string& user_id);
    crow::response getUnbanTime(const std::string& user_id) const;
    crow::response deleteUser(const std::string& id);
    crow::response getUserEnrollments(const std::string& id);
    crow::response getUserQueuedSlots(const std::string& id);

    template<typename AppType>
    void registerRoutes(AppType &app) {
        CROW_ROUTE(app, "/v1/user/<string>/gainedHours")(
            [this, &app](const crow::request &req, const std::string &user_id) {
                const auto &ctx = app.template get_context<AuthMiddleware>(req);
                if (!ctx.is_admin && ctx.user_id != user_id) {
                    return ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Cannot access another user").build();
                }
                return getUserHours(user_id);
            });
        CROW_ROUTE(app, "/v1/user/<string>/gainedHours")
                .methods("POST"_method)(
                    [this, &app](const crow::request &req, const std::string &user_id) {
                        const auto &ctx = app.template get_context<AuthMiddleware>(req);
                        if (!ctx.is_admin) {
                            return ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Admin access required").build();
                        }
                        return addUserHours(req, user_id);
                    });

        CROW_ROUTE(app, "/v1/user/<string>/isBanned")
                .methods("POST"_method)(
                    [this, &app](const crow::request &req, const std::string &user_id) {
                        const auto &ctx = app.template get_context<AuthMiddleware>(req);
                        if (!ctx.is_admin) {
                            return ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Admin access required").build();
                        }
                        return banUser(req, user_id);
                    });

        CROW_ROUTE(app, "/v1/user/<string>/unbanTime")(
            [this, &app](const crow::request &req, const std::string &user_id) {
                const auto &ctx = app.template get_context<AuthMiddleware>(req);
                if (!ctx.is_admin && ctx.user_id != user_id) {
                    return ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Cannot access another user").build();
                }
                return getUnbanTime(user_id);
            });

        CROW_ROUTE(app, "/v1/user/<string>/stats")(
            [this, &app](const crow::request &req, const std::string &user_id) {
                const auto &ctx = app.template get_context<AuthMiddleware>(req);
                if (!ctx.is_admin && ctx.user_id != user_id) {
                    return ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Cannot access another user").build();
                }
                // TODO: not mvp
                return ResponseBuilder().build();
            });

        CROW_ROUTE(app, "/v1/user/<string>/account")
                .methods("DELETE"_method)(
                    [this, &app](const crow::request &req, const std::string &id) {
                        const auto &ctx = app.template get_context<AuthMiddleware>(req);
                        if (!ctx.is_admin && ctx.user_id != id) {
                            return ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Cannot delete another user").build();
                        }
                        return deleteUser(id);
                    });

        CROW_ROUTE(app, "/v1/user/<string>/enrollments").methods("GET"_method)(
            [this, &app](const crow::request& req, const std::string& user_id) {
                const auto &ctx = app.template get_context<AuthMiddleware>(req);
                if (!ctx.is_admin && ctx.user_id != user_id) {
                    return ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Cannot access another user").build();
                }
                return getUserEnrollments(user_id);
            });

        CROW_ROUTE(app, "/v1/user/<string>/queuedSlots").methods("GET"_method)(
            [this, &app](const crow::request& req, const std::string& user_id) {
                const auto &ctx = app.template get_context<AuthMiddleware>(req);
                if (!ctx.is_admin && ctx.user_id != user_id) {
                    return ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Cannot access another user").build();
                }
                return getUserQueuedSlots(user_id);
            });
    }

private:
    std::shared_ptr<Database> db;
    UserManager db_user;
};

#endif // TRUTEN_SERVER_USER_ROUTES_HPP
