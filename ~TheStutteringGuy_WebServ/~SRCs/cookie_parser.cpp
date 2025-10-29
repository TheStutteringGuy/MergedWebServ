#include "WebServer.hpp"

std::string Client::trim_whitespaces(const std::string &str)
{
    const std::string whitespace = "\t\n\r";

    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos)
        return "";

    size_t end = str.find_last_not_of(whitespace);

    return (str.substr(start, end - start + 1));
};

void Client::parse_cookie()
{
    const std::vector<std::string> *cookie_header = find_Value_inMap(this->m_request.m_headers, "Cookie");

    if (NULL == cookie_header || (*cookie_header).empty())
        return;

    std::string row_cookie_header = (*cookie_header)[0];
    std::string delimiter = "; ";
    size_t pos;

    while (true)
    {
        pos = row_cookie_header.find(delimiter);
        std::string cookie_pair;

        if (pos == std::string::npos)
            cookie_pair = row_cookie_header;
        else
            cookie_pair = row_cookie_header.substr(0, pos);

        size_t equal_pos = cookie_pair.find('=');
        if (equal_pos != std::string::npos)
        {
            std::string key = cookie_pair.substr(0, equal_pos);
            std::string value = cookie_pair.substr(equal_pos + 1);

            std::string trimmed_key = trim_whitespaces(key);
            std::string trimmed_value = trim_whitespaces(value);

            if (!trimmed_key.empty())
                this->m_request.m_cookie[trimmed_key] = trimmed_value;

            if (pos == std::string::npos)
                break;

            row_cookie_header.erase(0, pos + delimiter.size());
        }
    }
}

std::string Client::serialize_cookies() const
{
    std::string cookie_string;

    for (std::map<std::string, std::string>::const_iterator it = this->m_request.m_cookie.begin(); it != this->m_request.m_cookie.end(); it++)
    {
        cookie_string += it->first;
        cookie_string += "=";
        cookie_string += it->second;

        if (++it != this->m_request.m_cookie.end())
            cookie_string += "; ";
        --it;
    }
    return (cookie_string);
}