#include "WebServer.hpp"

std::string sessionManager::generate_session_id()
{
    std::ostringstream oss;
    oss << std::rand() << std::time(NULL);
    return (oss.str());
};

std::string sessionManager::create_session(const std::string &user_id)
{
    std::string new_id;
    new_id = generate_session_id();
    session new_session;

    new_session.user_id = user_id;
    new_session.experation_time = std::time(NULL) + timeout_sec;
    m_session[new_id] = new_session;
    return (new_id);
};

bool sessionManager::get_session(const std::string &session_id, session &out_session)
{
    std::map<std::string, session>::iterator it = m_session.find(session_id);
    if (it == m_session.end())
        return (false);

    if (it->second.experation_time < std::time(NULL))
    {
        m_session.erase(it);
        return (false);
    }

    it->second.experation_time = std::time(NULL) + timeout_sec;
    out_session = it->second;

    return (true);
};

void sessionManager::cleanup_session()
{
    time_t curr_time = std::time(NULL);

    std::map<std::string, session>::iterator it = m_session.begin();
    while (it != m_session.end())
    {
        if (it->second.experation_time < curr_time)
        {
            std::map<std::string, session>::iterator to_erase = it;
            ++it;
            m_session.erase(to_erase);
        }
        else
            ++it;
    }
};

void Client::handle_session_logic()
{
    this->parse_cookie();

    std::string session_cookie_value;
    std::map<std::string, std::string>::const_iterator it = this->m_request.m_cookie.find("session_id");
    if (it != this->m_request.m_cookie.end())
        session_cookie_value = it->second;

    sessionManager &sm = sessionManager::getSession();
    if (!session_cookie_value.empty())
    {
        if (sm.get_session(session_cookie_value, this->session_data))
        {
            this->session_id = session_cookie_value;
            this->m_session_needs_set_cookie = false;
            this->m_request.m_cookie["session_id"] = this->session_id;
            return;
        }
    }
    this->session_id = sm.create_session();
    sm.get_session(this->session_id, this->session_data);
    this->m_session_needs_set_cookie = true;
    this->m_request.m_cookie["session_id"] = this->session_id;
};

std::string Client::get_session_cookie_header()
{
    if (this->m_session_needs_set_cookie == true)
    {
        std::ostringstream oss;
        oss << sessionManager::getSession().getTimeout();
        std::string max_age_str = oss.str();

        std::string header = "Set-Cookie: session_id=" + this->session_id +
                             "; Path=/; HttpOnly; Max-Age=" +
                             max_age_str + "\r\n";
        return (header);
    }
    return ("");
};