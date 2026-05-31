#include "response_builder.hpp"
#include "crow.h"
#include "status_codes.hpp"


ResponseBuilder::ResponseBuilder(const int code,
                                 const std::optional<std::string> &error_message)
    : status_code(code) {
    if (error_message.has_value() && !error_message->empty()) {
        addField("error", *error_message);
        return;
    }
    if (code != *RESPONSE_CODE::OK && code != *RESPONSE_CODE::OK_EMPTY) {
        changeStatusCode(code);
    }
}

void ResponseBuilder::changeStatusCode(const int new_code) {
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

crow::response ResponseBuilder::build() {
    //for whatever reason QT treats empty responses as errors.
    //gotta think of a workaround. ATM this works.
    if (status_code == *RESPONSE_CODE::OK || status_code == *RESPONSE_CODE::OK_EMPTY) {
        response_body["success"] = true;
    }
    return {status_code, response_body.dump()};
}
