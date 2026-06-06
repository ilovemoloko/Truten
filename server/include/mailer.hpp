#ifndef TRUTEN_MAILER_HPP
#define TRUTEN_MAILER_HPP

#include <string>

struct Mailer {
    Mailer(std::string host, int port, std::string user, std::string pass, std::string from_addr);

    bool send(const std::string &to, const std::string &subject, const std::string &body) const;

private:
    std::string m_host;
    int m_port;
    std::string m_user;
    std::string m_pass;
    std::string m_from;
};

#endif
