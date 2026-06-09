#include "scheduler.hpp"
#include "database.hpp"
#include "slot_manager.hpp"
#include "user_manager.hpp"
#include "mailer.hpp"
#include <thread>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <iostream>
#include <pqxx/pqxx>

void Scheduler::runWeeklyReset(std::shared_ptr<Database> db) {
    using namespace std::chrono;
    constexpr auto msk_offset = hours(3);
    sys_days last_reset;

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(30));

        auto now = system_clock::now();
        auto now_msk = now + msk_offset;
        auto tt = system_clock::to_time_t(now_msk);
        struct tm parts{};
        gmtime_r(&tt, &parts);

        int days_since_monday = (parts.tm_wday - 1 + 7) % 7;
        auto this_monday = floor<days>(now_msk) - days(days_since_monday);
        auto monday_11 = this_monday + hours(11);

        if (now_msk < monday_11 || this_monday <= last_reset)
            continue;

        std::cerr << "[scheduler] Resetting all slots (Monday 11:00 MSK trigger)...\n";
        try {
            SlotManager mgr(db);
            mgr.resetWeekly();
            last_reset = this_monday;
            std::cerr << "[scheduler] Slot reset complete.\n";
        } catch (const std::exception &e) {
            std::cerr << "[scheduler] Reset failed: " << e.what() << "\n";
        }
    }
}

void Scheduler::runNotifications(std::shared_ptr<Database> db) {
    const char *smtp_host = std::getenv("SMTP_HOST");
    const char *smtp_port = std::getenv("SMTP_PORT");
    const char *smtp_user = std::getenv("SMTP_USER");
    const char *smtp_pass = std::getenv("SMTP_PASS");
    const char *smtp_from = std::getenv("SMTP_FROM");

    if (!smtp_host || !smtp_user || !smtp_pass || !smtp_from) {
        std::cerr << "[notifier] SMTP not configured — skipping email notifications\n";
        return;
    }

    Mailer mailer(smtp_host,
                  smtp_port ? std::stoi(smtp_port) : 587,
                  smtp_user, smtp_pass, smtp_from);
    UserManager users(db);

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(30));

        try {
            struct SlotNotification {
                std::string slot_id;
                std::string start_time;
                std::string section;
                std::vector<std::string> user_ids;
            };
            std::vector<SlotNotification> notifications;

            auto slots = db->execute(
                "SELECT id, start_time, section_name FROM slots "
                "WHERE start_time BETWEEN NOW() + INTERVAL '55 minutes' "
                "AND NOW() + INTERVAL '65 minutes' "
                "AND is_cancelled = FALSE");

            for (auto row : slots) {
                std::string slot_id = row[0].as<std::string>();
                std::string start_time = row[1].as<std::string>();
                std::string section = row[2].as<std::string>();

                auto enrolled = db->execute(
                    "SELECT enrolled FROM slots WHERE id = $1", slot_id)[0][0];
                if (enrolled.is_null()) continue;

                auto arr = enrolled.as_array();
                std::pair<pqxx::array_parser::juncture, std::string> elem;

                std::vector<std::string> user_ids;
                do {
                    elem = arr.get_next();
                    if (elem.first == pqxx::array_parser::juncture::string_value)
                        user_ids.push_back(elem.second);
                } while (elem.first != pqxx::array_parser::juncture::done);

                notifications.push_back({std::move(slot_id), std::move(start_time), std::move(section), std::move(user_ids)});
            }

            std::string subject = "Truten — напоминание о занятии";
            for (auto &slot : notifications) {
                std::string body = "Здравствуйте!\n\n"
                    "Через час у вас занятие в \"" + slot.section + "\" в " + slot.start_time + ".\n\n"
                    "Хорошей тренировки!";

                for (const auto &uid : slot.user_ids) {
                    try {
                        std::string email = users.getEmailById(uid);
                        std::cerr << "[notifier] Sending reminder to "
                                  << email << " for slot " << slot.slot_id << "\n";
                        mailer.send(email, subject, body);
                    } catch (const std::exception &e) {
                        std::cerr << "[notifier] Failed for user " << uid
                                  << ": " << e.what() << "\n";
                    }
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "[notifier] Error: " << e.what() << "\n";
        }
    }
}
