#include "WebServer.hpp"
#include <ios>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define TIMEOUT_SEC 30000000

void _clear(Client &_client)
{
    std::map<int, Client> &client_map = ValuesSingleton::getValuesSingleton()._clients_map;

    close(_client.m_client_fd);
    epoll_ctl(ValuesSingleton::getValuesSingleton().epoll_fd, EPOLL_CTL_DEL, _client.m_client_fd, NULL);
    client_map.erase(_client.m_client_fd);
}

bool manage_Headers(CGIs& CGItohandle)
{
    std::string delimeter = "\r\n\r\n";

    size_t pos = CGItohandle.m_recv_buffer.find(delimeter);
    if (pos == std::string::npos)
        return false;
    CGItohandle.m_headers_buffer = CGItohandle.m_recv_buffer.substr(0, pos + delimeter.size());
    CGItohandle.m_body_buffer = CGItohandle.m_recv_buffer.substr(pos + delimeter.size());
    CGItohandle.m_recv_buffer.clear();
    return true;
}

void ManagingCGI(CGIs& CGItohandle, Client& _client, www::fd_t& CGIfd)
{
    (void)CGItohandle;
    (void)_client;

    char buffer[ReadingSize];
    memset(buffer, 0, sizeof(buffer));
    ssize_t read_bytes = recv(CGIfd, buffer, sizeof(buffer), MSG_DONTWAIT);

    if (CGItohandle.Header_sent == false)
    {
        if (read_bytes == 0 || read_bytes == -1)
        {
            int status;
            waitpid(CGItohandle.CGIpid, &status, WNOHANG);
            CGIManagerSingleton::getCGIManagerSingleton().CGIerase(CGIfd);
            _client.response_Error(500, true);
        }
        if (read_bytes > 0)
        {
            CGItohandle.m_recv_buffer.append(buffer, read_bytes);

            bool Headers_retrieved = manage_Headers(CGItohandle);
            if (Headers_retrieved == false)
                return;

            std::string Header = headers_Creator(Response("HTTP/1.1", 200, false, "", 0), 0);
            Header += "Transfer-Encoding: chunked\r\n";
            send(CGItohandle.client_fd, Header.c_str(), Header.size(), MSG_DONTWAIT);
            send(CGItohandle.client_fd, CGItohandle.m_headers_buffer.c_str(), CGItohandle.m_headers_buffer.size(), MSG_DONTWAIT);
            CGItohandle.Header_sent = true;

            std::stringstream ss;
            std::string size;
            ss << std::hex << CGItohandle.m_body_buffer.size();
            size = ss.str();

            std::string end = "\r\n";

            send(CGItohandle.client_fd, size.c_str(), size.size(), MSG_DONTWAIT);
            send(CGItohandle.client_fd, end.c_str(), end.size(), MSG_DONTWAIT);

            send(CGItohandle.client_fd, CGItohandle.m_body_buffer.c_str(), CGItohandle.m_body_buffer.size(), MSG_DONTWAIT);
            send(CGItohandle.client_fd, end.c_str(), end.size(), MSG_DONTWAIT);
        }
    }
    else 
    {
        if (read_bytes == 0 || read_bytes == -1)
        {
            int status;
            waitpid(CGItohandle.CGIpid, &status, WNOHANG);

            std::string theEnd = "0\r\n\r\n";
            send(CGItohandle.client_fd, theEnd.c_str(), theEnd.size(), MSG_DONTWAIT);
            CGIManagerSingleton::getCGIManagerSingleton().CGIerase(CGIfd);
            _clear(_client);
        }
        if (read_bytes > 0)
        {
            std::string body = buffer;
            std::string end = "\r\n";


            std::stringstream ss;
            std::string size;
            ss << std::hex << body.size();
            size = ss.str();

            send(CGItohandle.client_fd, size.c_str(), size.size(), MSG_DONTWAIT);
            send(CGItohandle.client_fd, end.c_str(), end.size(), MSG_DONTWAIT);

            send(CGItohandle.client_fd, body.c_str(), body.size(), MSG_DONTWAIT);
            send(CGItohandle.client_fd, end.c_str(), end.size(), MSG_DONTWAIT);
        }
    }
}

void multiplexer(void)
{
    www::fd_t &epoll_fd = ValuesSingleton::getValuesSingleton().epoll_fd;
    std::map<int, Client> &client_map = ValuesSingleton::getValuesSingleton()._clients_map;
    std::map<www::fd_t, MyServerBlock>  &serverfd_map = ValuesSingleton::getValuesSingleton()._serverfd_map;
    std::vector<www::fd_t> &_CGIfds_vect = CGIManagerSingleton::getCGIManagerSingleton()._CGIfds_vect;

    size_t timeout = getTime();
    std::map<int, Client>::iterator it = client_map.begin();
    while (it != client_map.end())
    {
        if ((timeout - it->second.m_connectedTime) >= TIMEOUT_SEC)
        {
            std::map<int, Client>::iterator temp_it = it;
            ++it;
            epoll_ctl(ValuesSingleton::getValuesSingleton().epoll_fd, EPOLL_CTL_DEL, temp_it->first, NULL);
            close(temp_it->first);
            client_map.erase(temp_it);
        }
        else
            ++it;
    }

    epoll_event events[MAX_EVENTS];
    int n = epoll_wait(epoll_fd, events, MAX_EVENTS, 10000);
    if (-1 == n)
    {
        if (errno == EINTR)
            throw www::SHUTDOWN;
        throw std::logic_error("epoll_wait() " + static_cast<std::string>(strerror(errno)));
    }

    if (0 == n)
        return;
    for (int index = 0; index < n && www::SHUTDOWN == false; ++index)
    {
        if (serverfd_map.find(events[index].data.fd) != serverfd_map.end())
        {
            int client_sfd = accept(events[index].data.fd, NULL, NULL);
            if (client_sfd == -1)
                throw std::runtime_error("accept()" + static_cast<std::string>(strerror(errno)));
            std::cout << "[INFO] : (A Client Connected) fd Reserved = "<< client_sfd << std::endl;

            epoll_event event;
            event.events = EPOLLIN | EPOLLHUP | EPOLLERR;
            event.data.fd = client_sfd;

            Client temp(client_sfd);
            temp.m_Myserver = serverfd_map[events[index].data.fd];
            temp.m_connectedTime = getTime();
            client_map[client_sfd] = temp;

            if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sfd, &event))
                throw std::logic_error("epoll_ctl() " + static_cast<std::string>(strerror(errno)));
        }
        else
        {
            // CGI STUFF :
            if (std::find(_CGIfds_vect.begin(), _CGIfds_vect.end(), events[index].data.fd) != _CGIfds_vect.end())
            {
                CGIs& CGItohandle = CGIManagerSingleton::getCGIManagerSingleton().CGIsMap[events[index].data.fd];
                Client& _client = client_map[CGItohandle.client_fd];

                if (events[index].events & EPOLLIN)
                {
                    ManagingCGI(CGItohandle, _client, events[index].data.fd);
                    continue;
                }
                    
                if (events[index].events & EPOLLERR || events[index].events & EPOLLHUP)
                {
                    if (CGItohandle.Header_sent == true)
                    {
                        int status;
                        waitpid(CGItohandle.CGIpid, &status, WNOHANG);

                        std::string theEnd = "0\r\n\r\n";
                        send(CGItohandle.client_fd, theEnd.c_str(), theEnd.size(), MSG_DONTWAIT);
                        CGIManagerSingleton::getCGIManagerSingleton().CGIerase(events[index].data.fd);
                        _clear(_client);
                    }
                    else 
                    {
                        int status;
                        waitpid(CGItohandle.CGIpid, &status, WNOHANG);
                        CGIManagerSingleton::getCGIManagerSingleton().CGIerase(events[index].data.fd);
                        _client.response_Error(500, true);
                    }
                }
                continue;
            }

            if (events[index].events & EPOLLERR || events[index].events & EPOLLHUP)
            {
                std::cout << "[INFO] : (A Client Disconnected with an event other than EPOLLIN) fd Reserved = "<< events[index].data.fd << std::endl;
                _clear(client_map[events[index].data.fd]);
                continue;
            }
            try
            {
                Client &_client = client_map[events[index].data.fd];
                _client.m_connectedTime = getTime();

                if (_client.readyto_send == false)
                {
                    char buffer[ReadingSize];
                    memset(buffer, 0, sizeof(buffer));
                    ssize_t bytes_read = recv(_client.m_client_fd, buffer, sizeof(buffer), MSG_DONTWAIT);
                    
                    if (-1 == bytes_read)
                    {
                        _clear(_client);
                        throw std::runtime_error("recv() " + static_cast<std::string>(strerror(errno)));
                    }
                    else if (0 == bytes_read)
                    {
                        std::cout << "[INFO] : (A Client Disconnected) fd Reserved = : "<< events[index].data.fd << std::endl;
                        _clear(_client);
                        continue;
                    }
                    else if (0 < bytes_read)
                    {
                        _client.initialize_body_asFile();
                        
                        if (_client.handling_request == false)
                        {
                            if (_client.header_done == false)
                                _client.m_request_buffer.append(buffer, bytes_read);
                            else
                                _client.m_body_buffer.append(buffer, bytes_read);
            
                            if (_client.header_done == false)
                            {
                                if (true == _client.headerEnd_Checker())
                                    continue;
                                _client.headers_parser();
                                _client.headers_implication();
                            }

                            if (_client.need_body == true)
                            {
                                if (true == _client.body_checker())
                                    continue;
                            }
                        }
                        _client.handle_Request();
                    }
                }
                _client.handle_Response();
            }
            catch (std::logic_error &e)
            {
                throw e;
            }
            catch (std::runtime_error &e)
            {
                std::cerr << www::RED << "[ERROR] : " << e.what() << www::RESET << std::endl;
                continue;
            }
            catch (std::exception &e)
            {
                std::cerr << www::YELLOW << "[DEBUG] : " << e.what() << www::RESET << std::endl;
                continue;
            }
            catch (const int &i)
            {
                switch (i) {
                    case CONTINUE: continue;
                    case END: { std::cout << "[INFO] : Closing the Connection to the Client by My side" << std::endl ; continue; }
                    default: continue;
                }
            }
        }
    }
}