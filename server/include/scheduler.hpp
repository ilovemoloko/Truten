#ifndef TRUTEN_SCHEDULER_HPP
#define TRUTEN_SCHEDULER_HPP

#include <memory>

struct Database;

struct Scheduler {
    static void runWeeklyReset(std::shared_ptr<Database> db);
    static void runNotifications(std::shared_ptr<Database> db);
};

#endif
