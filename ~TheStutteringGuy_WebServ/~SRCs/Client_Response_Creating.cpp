#include "WebServer.hpp"
#include <exception>

void Client::response_Error(const unsigned int &error_code, bool Content_needed)
{
    std::string body;
    std::string Headers;

    try 
    {
        if (this->m_Myserver.m_error_pages.find(error_code) != this->m_Myserver.m_error_pages.end())
            this->response_Get(this->m_Myserver.m_root + this->m_Myserver.m_error_pages[error_code]);
    }
    catch (int &i) { i == CONTINUE ? throw CONTINUE : throw END;}
    catch (...) {}

    if (Content_needed == true)
    {
        body = generic_error_page(error_code, ValuesSingleton::getValuesSingleton().m_HTTPstatus_phrase.getStatusPhrase(to_string_c98(error_code)));
        Headers = headers_Creator(Response("HTTP/1.0", error_code, true, "text/html", body.size()));
    }
    else
        Headers = headers_Creator(Response("HTTP/1.0", error_code, false, "text/html", body.size()));

    std::string to_send = Headers + body;
    this->m_response_buffer = to_send;
    this->readyto_send = true;

    epoll_event event;
    event.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
    event.data.fd = this->m_client_fd;
    epoll_ctl(ValuesSingleton::getValuesSingleton().epoll_fd, EPOLL_CTL_MOD, this->m_client_fd, &event);
    throw CONTINUE;
}

void Client::response_html_ready(const std::string &string_)
{
    std::string Headers = headers_Creator(Response("HTTP/1.0", 200, true, "text/html", string_.size()));

    std::string to_send = Headers + string_;
    this->m_response_buffer = to_send;
    this->readyto_send = true;

    epoll_event event;
    event.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
    event.data.fd = this->m_client_fd;
    epoll_ctl(ValuesSingleton::getValuesSingleton().epoll_fd, EPOLL_CTL_MOD, this->m_client_fd, &event);
    throw CONTINUE;
}

///////////////////////////////////////////////////////////////////////////////////////////

void Client::response_Get(const std::string& File)
{
    std::string l_Content_type;
    std::string body;

    std::fstream file(File.c_str(), std::ios::in | std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("file.open()" + static_cast<std::string>(strerror(errno)));

    size_t size = file.tellg();
    size_t pos;
    pos = File.find_last_of(".");
    if (pos != std::string::npos)
    {
        std::string file_extension = File.substr(pos + 1);
        l_Content_type = ValuesSingleton::getValuesSingleton().m_FileContent_type.getContent_type(file_extension);
    }
    else
        l_Content_type = ValuesSingleton::getValuesSingleton().m_FileContent_type.getContent_type(std::string());

    std::string Headers = headers_Creator(Response("HTTP/1.0", 200, true,  l_Content_type, size));
    this->m_response_buffer = Headers;
    this->readyto_send = true;

    this->m_response_is_aFile = true;
    this->initialize_response_asFile(File);

    epoll_event event;
    event.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
    event.data.fd = this->m_client_fd;
    epoll_ctl(ValuesSingleton::getValuesSingleton().epoll_fd, EPOLL_CTL_MOD, this->m_client_fd, &event);

    throw CONTINUE;
}

void Client::response_justAstatus(const unsigned int &status_code)
{
    std::string Headers = headers_Creator(Response("HTTP/1.0", status_code, false,  std::string(), 0));
    this->m_response_buffer = Headers;
    this->readyto_send = true;

    epoll_event event;
    event.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
    event.data.fd = this->m_client_fd;
    epoll_ctl(ValuesSingleton::getValuesSingleton().epoll_fd, EPOLL_CTL_MOD, this->m_client_fd, &event);
    throw CONTINUE;
}