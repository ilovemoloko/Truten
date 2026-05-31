#include "crow.h"
#include "database.hpp"
#include "jwt.hpp"
#include "router.hpp"
#include <cstdlib>

int main() {
    // "../.env" because of cmake-build-debug
    Database::loadEnv("../.env");

    // NOTE: you need .env LOCAL file in format like this
    // DB_HOST=localhost
    // DB_PORT=5432
    // DB_NAME=db_name
    // DB_USER=some_user
    // DB_PASS=pass
    // put .env next to CMakeList.txt

    const char* host = std::getenv("DB_HOST");
    const char* port = std::getenv("DB_PORT");
    const char* dbname = std::getenv("DB_NAME");
    const char* user = std::getenv("DB_USER");
    const char* pass = std::getenv("DB_PASS");

    const std::string connection_string =
        "host=" + std::string(host ? host : "localhost") + " " +
        "port=" + std::string(port ? port : "5432") + " " +
        "dbname=" + std::string(dbname ? dbname : "testdb") + " " +
        "user=" + std::string(user ? user : "postgres") + " " +
        "password=" + std::string(pass ? pass : "");

    std::cerr << "Connecting to DB on " << (host ? host : "localhost") << "...\n";

    auto db = std::make_shared<Database>(connection_string);
    crow::App<AuthMiddleware> app;
    ServerMain router(app, db);
    router.run();
    return 0;
}
