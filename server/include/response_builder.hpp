#ifndef SERVER_RESPONSE_BUILDER_HPP
#define SERVER_RESPONSE_BUILDER_HPP
#include "crow.h"
#include "request_handler.hpp"
#include "status_codes.hpp"

struct ResponseBuilder {
public:
    ResponseBuilder() : status_code(*RESPONSE_CODE::OK) {
    }

    explicit ResponseBuilder(const int code,
                             const std::optional<std::string> &error_message = std::nullopt)
        : status_code(code) {
        if (error_message.has_value() && !error_message->empty()) {
            addField("error", *error_message);
            return;
        }
        if (code != *RESPONSE_CODE::OK && code != *RESPONSE_CODE::OK_EMPTY) {
            changeStatusCode(code);
        }
    }

    explicit ResponseBuilder(const RESPONSE_CODE c,
                             const std::string &error_message = "")
        : ResponseBuilder(*c, error_message) {
    }

    explicit ResponseBuilder(const RequestHandler &request)
        : ResponseBuilder(request.response_code, request.error_message) {
    }

    void changeStatusCode(const int new_code) {
        status_code = new_code;
        if (new_code == *RESPONSE_CODE::NO_ACCESS) {
            addField("error", "You can't access this method");
        } else if (new_code == *RESPONSE_CODE::SERVER_ERROR) {
            addField("error", "Something broke on the server :(");
        } else if (new_code == *RESPONSE_CODE::INVALID) {
            addField("error", "Some values you provided are invalid");
        } else if (new_code == *RESPONSE_CODE::NOT_FOUND) {
            addField("error", "Couldn't find what you asked for!");
        }
    }

    void changeStatusCode(const RESPONSE_CODE new_code) {
        changeStatusCode(*new_code);
    }

    template<typename T>
    ResponseBuilder addField(const std::string &field_name, const T &value) {
        response_body[field_name] = value;
        return *this;
    }

    [[nodiscard]] crow::response build() {
        return {status_code, response_body.dump()};
    }

private:
    int status_code;
    crow::json::wvalue response_body;
};

#endif // SERVER_RESPONSE_BUILDER_HPP
