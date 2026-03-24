#ifndef TRUTEN_SERVER_ROUTER_HPP
#define TRUTEN_SERVER_ROUTER_HPP

#include "auth_routes.hpp"
#include "crow.h"
#include "database.hpp"
#include "queue_routes.hpp"
#include "section_routes.hpp"
#include "slot_routes.hpp"
#include "user_routes.hpp"

struct ServerMain {
    explicit ServerMain(const crow::SimpleApp &orig_app, std::shared_ptr<Database> orig_db)
        : app(std::move(orig_app)), db(std::move(orig_db)) {
    }

    void run() {
        /*
         *I didn't want to make a constructor
         *like AuthRoutes auth(db, app)
         *because I believe that would be
         *misleading. You don't expect
         *something to change when simply
         *creating an object.
         */
        db->init();
        AuthRoutes auth(db);
        QueueRoutes queue;
        //SectionRoutes sections; double "/v1/sections/gymList" implementation
        SlotRoutes slots(db);
        UserRoutes users(db);
        auth.registerRoutes(app);
        queue.registerRoutes(app);
        //sections.registerRoutes(app);
        slots.registerRoutes(app);
        users.registerRoutes(app);
        app.port(8080).multithreaded().run();
    }

private:
    crow::SimpleApp app;
    std::shared_ptr<Database> db;
};

#endif // TRUTEN_SERVER_ROUTER_HPP
