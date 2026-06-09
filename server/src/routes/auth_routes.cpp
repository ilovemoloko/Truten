#include "auth_routes.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <iomanip>
#include <sstream>

static std::string toHex(const unsigned char* data, size_t length) {
    std::stringstream ss;
    for(size_t i = 0; i < length; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
    return ss.str();
}

static int hexCharToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static bool fromHex(const std::string& hex, unsigned char* out, size_t out_len) {
    if (hex.length() != out_len * 2) return false;
    for (size_t i = 0; i < out_len; ++i) {
        int hi = hexCharToInt(hex[i * 2]);
        int lo = hexCharToInt(hex[i * 2 + 1]);
        if (hi < 0 || lo < 0) return false;
        out[i] = (hi << 4) | lo;
    }
    return true;
}

static std::string hashPassword(const std::string& password) {
    unsigned char salt[16];
    RAND_bytes(salt, sizeof(salt));
    
    unsigned char hash[32];
    PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                      salt, sizeof(salt),
                      10000,
                      EVP_sha256(),
                      sizeof(hash), hash);
                      
    return toHex(salt, sizeof(salt)) + ":" + toHex(hash, sizeof(hash));
}

static bool verifyPassword(const std::string& password, const std::string& stored) {
    auto colon_pos = stored.find(':');
    if (colon_pos == std::string::npos) return false;
    
    std::string salt_hex = stored.substr(0, colon_pos);
    std::string hash_hex = stored.substr(colon_pos + 1);
    
    unsigned char salt[16];
    unsigned char expected_hash[32];
    
    if (!fromHex(salt_hex, salt, sizeof(salt)) || 
        !fromHex(hash_hex, expected_hash, sizeof(expected_hash))) {
        return false;
    }
    
    unsigned char hash[32];
    PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                      salt, sizeof(salt),
                      10000,
                      EVP_sha256(),
                      sizeof(hash), hash);
                      
    return CRYPTO_memcmp(hash, expected_hash, sizeof(hash)) == 0;
}

AuthRoutes::AuthRoutes(const std::shared_ptr<Database>& db) : db_(db), db_auth(db), db_user(db) {
}

crow::response AuthRoutes::createAccount(const crow::request &req) {
    RequestHandler request(req);
    request.require("email", crow::json::type::String);
    request.require("password", crow::json::type::String);
    request.require("name", crow::json::type::String);
    if (!request.responseIsOk()) {
        return ResponseBuilder(request).build();
    }
    const auto email = static_cast<std::string>(request["email"]);
    const auto password = static_cast<std::string>(request["password"]);
    const auto name = static_cast<std::string>(request["name"]);
    
    return db_->executeInTransaction([&]() {
        if (db_auth.emailExists(email, true)) {
            return ResponseBuilder(RESPONSE_CODE::INVALID).build();
        }
        db_auth.createUser(email, hashPassword(password), name);
        const std::string user_id = db_auth.getUserIdByEmail(email);
        const std::string access_token = make_access_token(user_id, false);
        const std::string refresh_token = make_refresh_token(user_id);
        ResponseBuilder resp;
        resp.addField("userId", user_id);
        resp.addField("isAdmin", false);
        resp.addField("accessToken", access_token);
        resp.addField("refreshToken", refresh_token);
        return resp.build();
    });
}

crow::response AuthRoutes::login(const crow::request &req) const {
    RequestHandler request(req);
    request.require("email", crow::json::type::String);
    request.require("password", crow::json::type::String);
    if (!request.responseIsOk()) {
        return ResponseBuilder(request).build();
    }
    const auto email = static_cast<std::string>(request["email"]);
    const auto password = static_cast<std::string>(request["password"]);
    
    return db_->executeInTransaction([&]() {
        const std::string real_password = db_auth.getPasswordByEmail(email);
        const std::string user_id = db_auth.getUserIdByEmail(email);
        if (!verifyPassword(password, real_password)) {
            return ResponseBuilder(RESPONSE_CODE::NO_ACCESS).build();
        }
        const bool is_admin = db_user.isAdmin(user_id);
        const std::string access_token = make_access_token(user_id, is_admin);
        const std::string refresh_token = make_refresh_token(user_id);
        ResponseBuilder resp;
        resp.addField("userId", user_id);
        resp.addField("isAdmin", is_admin);
        resp.addField("accessToken", access_token);
        resp.addField("refreshToken", refresh_token);
        return resp.build();
    });
}

crow::response AuthRoutes::refresh(const crow::request &req) const {
    RequestHandler request(req);
    request.require("refreshToken", crow::json::type::String);
    if (!request.responseIsOk()) {
        return ResponseBuilder(request).build();
    }
    const auto token_str = static_cast<std::string>(request["refreshToken"]);
    try {
        auto decoded = jwt::decode(token_str);
        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{jwt_secret()})
            .verify(decoded);

        if (!decoded.has_payload_claim("typ") ||
            decoded.get_payload_claim("typ").as_string() != "refresh") {
            return ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Not a refresh token").build();
        }

        const std::string user_id = decoded.get_payload_claim("sub").as_string();
        return db_->executeInTransaction([&]() {
            const bool is_admin = db_user.isAdmin(user_id);
            const std::string new_access = make_access_token(user_id, is_admin);
            ResponseBuilder resp;
            resp.addField("accessToken", new_access);
            return resp.build();
        });
    } catch (...) {
        return ResponseBuilder(RESPONSE_CODE::NO_ACCESS, "Invalid refresh token").build();
    }
}