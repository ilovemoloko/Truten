//
// Created by asavelev on 2/17/26.
//

#ifndef SERVER_REQUEST_HANDLER_HPP
#define SERVER_REQUEST_HANDLER_HPP
#include <vector>
#include "crow.h"
#include "status_codes.hpp"

struct requestHandler {
    explicit requestHandler(const crow::request& req) : request(req), body(crow::json::load(req.body)){
    }

    void require(const std::string& field_name, const crow::json::type required_type = crow::json::type::Object) {
        if (!body.has(field_name)) {
            response_codes.push_back(*RESPONSE_CODE::INVALID);
            return;
        }
        if (required_type == crow::json::type::Object) return;
        if (body[field_name].t() != required_type) {
            response_codes.push_back(*RESPONSE_CODE::INVALID);
        }
    }

    auto operator[](std::string field_name) const {
        return body[field_name];
    }

    int firstResponse() const {
        return response_codes[0];
    }

private:
    std::vector<int> response_codes;
    crow::request request;
    crow::json::rvalue body;
};

#endif //SERVER_REQUEST_HANDLER_HPP