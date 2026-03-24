#include "crow.h"
#include "database.hpp"
#include "router.hpp"

int main() {
    const std::string connection_string =
            "dbname=testdb " // TODO: learn how to do this properly
            "user=postgres "
            "host=localhost "
            "port=5432";

    auto db = std::make_shared<Database>(connection_string);
    crow::SimpleApp app;
    ServerMain router(app, db);
    router.run();
    return 0;
}
