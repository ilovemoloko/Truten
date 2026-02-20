//
// Created by asavelev on 2/17/26.
//

#ifndef SERVER_REQUEST_HANDLER_HPP
#define SERVER_REQUEST_HANDLER_HPP
#include <vector>
#include "crow.h"
#include "status_codes.hpp"

struct ResponseBuilder;

struct requestHandler {
    friend struct ResponseBuilder;

    explicit requestHandler(const crow::request &req) : response_code(*RESPONSE_CODE::OK), request(req),
                                                        body(crow::json::load(req.body)) {
    }

    void require(const std::string &field_name, const crow::json::type required_type = crow::json::type::Object) {
        if (!body.has(field_name)) {
            response_code = *RESPONSE_CODE::INVALID;
            error_message = "Missing required field " + field_name;
            return;
        }
        if (required_type == crow::json::type::Object) return;
        if (body[field_name].t() != required_type) {
            response_code = *RESPONSE_CODE::INVALID;
            error_message = "Invalid type of " + field_name;
        }
    }

    [[nodiscard]] bool responseIsOk() const {
        return (response_code == 200 || response_code == 204);
    }

    [[nodiscard]] auto operator[](const std::string &field_name) const {
        return body[field_name];
    }

private:
    int response_code;
    std::string error_message;
    crow::request request;
    crow::json::rvalue body;
};

#endif //SERVER_REQUEST_HANDLER_HPP
