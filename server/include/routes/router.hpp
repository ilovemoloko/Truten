#ifndef TRUTEN_SERVER_ROUTER_HPP
#define TRUTEN_SERVER_ROUTER_HPP

#include "crow.h"
#include "database.hpp"
#include "jwt.hpp"
#include "auth_routes.hpp"
#include "queue_routes.hpp"
#include "section_routes.hpp"
#include "slot_routes.hpp"
#include "user_routes.hpp"

struct ServerMain {
    explicit ServerMain(crow::App<AuthMiddleware> &orig_app, std::shared_ptr<Database> orig_db)
        : app(orig_app), db(std::move(orig_db)) {
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
<<<<<<< Updated upstream
        QueueRoutes queue;
=======
        QueueRoutes queue(db);
>>>>>>> Stashed changes
        SectionRoutes sections(db);
        SlotRoutes slots(db);
        UserRoutes users(db);
        auth.registerRoutes(app);
        queue.registerRoutes(app);
        sections.registerRoutes(app);
        slots.registerRoutes(app);
        users.registerRoutes(app);
        app.port(8080).multithreaded().run();
    }

private:
    crow::App<AuthMiddleware> &app;
    std::shared_ptr<Database> db;
};

#endif // TRUTEN_SERVER_ROUTER_HPP
