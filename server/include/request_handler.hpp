#ifndef SERVER_REQUEST_HANDLER_HPP
#define SERVER_REQUEST_HANDLER_HPP
#include "crow.h"
struct ResponseBuilder;

struct RequestHandler {
    friend struct ResponseBuilder;

    explicit RequestHandler(const crow::request &req);

    void
    require(const std::string &field_name,
            crow::json::type required_type = crow::json::type::Object);

    [[nodiscard]] bool responseIsOk() const {
        return (response_code == 200 || response_code == 204);
    }

    [[nodiscard]] crow::json::rvalue
    operator[](const std::string &field_name) const {
        return body[field_name];
    }

private:
    int response_code;
    std::string error_message;
    crow::request request;
    crow::json::rvalue body;
};

#endif // SERVER_REQUEST_HANDLER_HPP
