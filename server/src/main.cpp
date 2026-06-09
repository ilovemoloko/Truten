#include "crow.h"
#include "database.hpp"
#include "jwt.hpp"
#include "router.hpp"
#include "scheduler.hpp"
#include <thread>
#include <cstdlib>

int main() {
    Database::loadEnv("../.env");
    Database::printEnvStatus();

    auto get_env_safe = [](const char* key) -> std::string {
        const char* val = std::getenv(key);
        return val ? val : "";
    };

    std::string host = get_env_safe("DB_HOST");
    std::string port = get_env_safe("DB_PORT");
    std::string dbname = get_env_safe("DB_NAME");
    std::string user = get_env_safe("DB_USER");
    std::string pass = get_env_safe("DB_PASS");

    const std::string connection_string =
        "host=" + (host.empty() ? "localhost" : host) + " " +
        "port=" + (port.empty() ? "5432" : port) + " " +
        "dbname=" + (dbname.empty() ? "testdb" : dbname) + " " +
        "user=" + (user.empty() ? "postgres" : user) + " " +
        "password=" + pass;

    std::cerr << "Connecting to DB...\n";

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
