#include "mailer.hpp"
#include <cstdlib>
#include <iostream>

Mailer::Mailer(std::string host, int port, std::string user, std::string pass, std::string from_addr)
    : m_host(std::move(host)), m_port(port), m_user(std::move(user)),
      m_pass(std::move(pass)), m_from(std::move(from_addr)) {
}

bool Mailer::send(const std::string &to, const std::string &subject, const std::string &body) const {
    setenv("SMTP_TO", to.c_str(), 0);
    setenv("SMTP_SUBJECT", subject.c_str(), 0);
    setenv("SMTP_BODY", body.c_str(), 0);
    setenv("SMTP_HOST", m_host.c_str(), 0);
    setenv("SMTP_PORT", std::to_string(m_port).c_str(), 0);
    setenv("SMTP_USER", m_user.c_str(), 0);
    setenv("SMTP_PASS", m_pass.c_str(), 0);
    setenv("SMTP_FROM", m_from.c_str(), 0);

    // python3 -c "..."  reads SMTP_* from env, no shell escaping issues
    const char *script =
        "python3 -c \"import os,smtplib,email.utils;"
        "from email.mime.text import MIMEText;"
        "msg=MIMEText(os.environ['SMTP_BODY']);"
        "msg['Subject']=os.environ['SMTP_SUBJECT'];"
        "msg['From']=os.environ['SMTP_FROM'];"
        "msg['To']=os.environ['SMTP_TO'];"
        "port=int(os.environ['SMTP_PORT']);"
        "s=(smtplib.SMTP_SSL if port==465 else smtplib.SMTP)(os.environ['SMTP_HOST'],port);"
        "port!=465 and s.starttls();"
        "s.login(os.environ['SMTP_USER'],os.environ['SMTP_PASS']);"
        "s.send_message(msg);"
        "s.quit()\"";

    int ret = std::system(script);
    if (ret != 0) {
        std::cerr << "[mailer] Failed to send email to " << to
                  << " (exit " << ret << ")\n";
        return false;
    }
    return true;
}
