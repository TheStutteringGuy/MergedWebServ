#include "WebServer.hpp"
#include <ios>

void Client::handle_Response(void)
{ 
    this->readyto_send = true;

    static bool flag;
    if (flag == false)
    {
        epoll_event event;
        event.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
        event.data.fd = this->m_client_fd;
        epoll_ctl(ValuesSingleton::getValuesSingleton().epoll_fd, EPOLL_CTL_MOD, this->m_client_fd, &event);
        flag = true;
    }

    static ssize_t bytes_sent;
    while (!this->m_response_buffer.empty())
    {
        ssize_t bytes_remained = this->m_response_buffer.size() - bytes_sent;

        ssize_t bytes_to_send = std::min(static_cast<ssize_t>(WritingSize), bytes_remained);
        std::string to_send = this->m_response_buffer.substr(0, bytes_to_send);
        this->m_response_buffer.erase(0, bytes_to_send);

        if (-1 == send(this->m_client_fd, to_send.c_str(), to_send.size(), MSG_DONTWAIT))
        {
            _clear(*this);
            throw std::runtime_error("send() "  + static_cast<std::string>(strerror(errno)));
        }
        throw CONTINUE;
    }
    if (this->m_response_is_aFile == true)
    {
        std::string buffer(WritingSize, 0);
        this->m_response_asFile->read(&buffer[0], WritingSize);
        std::streamsize bytes_read = this->m_response_asFile->gcount();

        if (bytes_read == 0)
        {
            _clear(*this);
            throw END;
        }
        
        ssize_t bytes_sent;
        if (-1 == (bytes_sent = send(this->m_client_fd, buffer.c_str(), bytes_read, MSG_DONTWAIT)))
        {
            _clear(*this);
            throw std::runtime_error("send() " + static_cast<std::string>(strerror(errno)));
        }

        if (bytes_sent < bytes_read)
            this->m_response_asFile->seekg(-(bytes_read - bytes_sent), std::ios::cur);

        throw CONTINUE;
    }
    _clear(*this);
    throw END;
}