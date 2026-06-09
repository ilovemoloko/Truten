#ifndef SERVER_RESPONSE_BUILDER_HPP
#define SERVER_RESPONSE_BUILDER_HPP
#include "crow.h"
#include "request_handler.hpp"
#include "status_codes.hpp"
#include <optional>

struct ResponseBuilder {
public:
    ResponseBuilder() : status_code(*RESPONSE_CODE::OK) {
    }

    explicit ResponseBuilder(int code,
                             const std::optional<std::string> &error_message = std::nullopt);

    explicit ResponseBuilder(const RESPONSE_CODE c,
                             const std::string &error_message = "")
        : ResponseBuilder(*c, error_message) {
    }

    explicit ResponseBuilder(const RequestHandler &request)
        : ResponseBuilder(request.response_code, request.error_message) {
    }

    void changeStatusCode(int new_code);

    void changeStatusCode(const RESPONSE_CODE new_code) {
        changeStatusCode(*new_code);
    }

    template<typename T>
    ResponseBuilder addField(const std::string &field_name, const T &value) {
        response_body[field_name] = value;

        return *this;
    }



    [[nodiscard]] crow::response build();

private:
    int status_code;
    crow::json::wvalue response_body;
};

#endif // SERVER_RESPONSE_BUILDER_HPP
