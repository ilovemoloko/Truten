//
// Created by asavelev on 2/17/26.
//

#ifndef SERVER_RESPONSE_BUILDER_HPP
#define SERVER_RESPONSE_BUILDER_HPP
#include "crow.h"
#include "status_codes.hpp"

struct requestHandler;

struct ResponseBuilder {
public:
    ResponseBuilder() {
        status_code = *RESPONSE_CODE::OK;
    }

    explicit ResponseBuilder(const int c, const std::string& error_message = "") {
        status_code = c;
        if (!error_message.empty()) {
            addField("error", error_message);
            return;
        }
        //i hate ts
        if (c == *RESPONSE_CODE::NO_ACCESS) {
            addField("error", "You can't access this method");
            return;
        }
        if (c == *RESPONSE_CODE::BAD_GATEWAY) {
            addField("error", "Something broke on the server :(");
            return;
        }
        if (c == *RESPONSE_CODE::INVALID) {
            addField("error", "Some values you provided are invalid");
            return;
        }
        if (c == *RESPONSE_CODE::NOT_FOUND) {
            addField("error", "Couldn't find what you asked for!");
            return;
        }
    }

    explicit ResponseBuilder(const RESPONSE_CODE c, const std::string& error_message = "") : ResponseBuilder(*c, error_message) {

    }

    explicit ResponseBuilder(const requestHandler& request) : ResponseBuilder(request.response_code, request.error_message) {

    }

    template<typename T>
    ResponseBuilder addField(const std::string& field_name, const T& value) {
        response_body[field_name] = value;
        return *this;
    }

    ResponseBuilder addField(const std::string& field_name, const std::vector<std::string>& value);

    [[nodiscard]] crow::response build() {
        return {status_code, response_body.dump()};
    }

private:
    int status_code;
    crow::json::wvalue response_body;
};

#endif //SERVER_RESPONSE_BUILDER_HPP