//
// Created by asavelev on 2/17/26.
//
#include "crow.h"
#include "database.hpp"
#include "router.hpp"

int main() {
    const std::string connection_string =
            "dbname=testdb " // TODO: learn how to do this properly
            "user=postgres "
            "host=localhost "
            "port=5432";

    Database db(connection_string);
    crow::SimpleApp app;
    Router router(app, db);
    router.setup_and_run();
    return 0;
}
