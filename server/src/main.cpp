#include "crow.h"
#include "database.hpp"
#include "jwt.hpp"
#include "router.hpp"
#include "scheduler.hpp"
#include <thread>
#include <cstdlib>

int main() {
    Database::loadEnv("../.env");

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

    std::thread scheduler_thread([db] { Scheduler::runWeeklyReset(db); });
    std::thread notifier_thread([db] { Scheduler::runNotifications(db); });

    crow::App<AuthMiddleware> app;
    ServerMain router(app, db);
    router.run();

    scheduler_thread.join();
    notifier_thread.join();
    return 0;
}
