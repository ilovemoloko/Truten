#include "request_handler.hpp"
#include "status_codes.hpp"

void RequestHandler::require(const std::string &field_name, crow::json::type required_type) {
    if (!body.has(field_name)) {
        response_code = *RESPONSE_CODE::INVALID;
        error_message = "Missing required field " + field_name;
        return;
    }
    if (required_type == crow::json::type::Object)
        return;
    if (body[field_name].t() != required_type) {
        response_code = *RESPONSE_CODE::INVALID;
        error_message = "Invalid type of " + field_name;
    }
}

RequestHandler::RequestHandler(const crow::request &req) : response_code(*RESPONSE_CODE::OK), request(req),
                                                           body(crow::json::load(req.body)) {
}
