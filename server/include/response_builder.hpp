//
// Created by asavelev on 2/17/26.
//

#ifndef SERVER_RESPONSE_BUILDER_HPP
#define SERVER_RESPONSE_BUILDER_HPP
#include "crow.h"
#include "status_codes.hpp"

struct responseBuilder {
public:
    responseBuilder() {
        status_code = *RESPONSE_CODE::OK;
    }

    explicit responseBuilder(const RESPONSE_CODE c) {
        status_code = *c;
    }

    explicit responseBuilder(const int c) {
        status_code = c;
    }

    void addField(const std::string& field_name, const int value) {
        response_body[field_name] = value;
    }

    void addField(const std::string& field_name, const std::string& value) {
        response_body[field_name] = value;
    }

    crow::response build() {
        return {status_code, response_body};
    }

private:
    int status_code;
    crow::json::wvalue response_body;
};

#endif //SERVER_RESPONSE_BUILDER_HPP