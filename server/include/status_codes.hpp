//
// Created by asavelev on 2/17/26.
//

#ifndef SERVER_STATUS_CODES_HPP
#define SERVER_STATUS_CODES_HPP

enum class RESPONSE_CODE {
    OK = 200,
    BAD_GATEWAY = 503,
    NOT_FOUND = 404,
    OK_EMPTY = 204,
    INVALID = 400,
    NO_ACCESS = 403
};

inline int operator*(RESPONSE_CODE code) { return static_cast<int>(code); }

#endif // SERVER_STATUS_CODES_HPP
