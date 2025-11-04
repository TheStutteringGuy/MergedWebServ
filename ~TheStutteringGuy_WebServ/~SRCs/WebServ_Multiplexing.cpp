#include "WebServer.hpp"

#define TIMEOUT_SIT 30000000
#define TIMEOUT_CGI 20000000

void _clear(Client &_client)
{
    std::map<int, Client> &client_map = ValuesSingleton::getValuesSingleton()._clients_map;

    if (client_map.find(_client.m_client_fd) == client_map.end())
        return;

    CGIManagerSingleton::getCGIManagerSingleton().CGIerase(_client.m_CGIfd);
    epoll_ctl(ValuesSingleton::getValuesSingleton().epoll_fd, EPOLL_CTL_DEL, _client.m_client_fd, NULL);
    close(_client.m_client_fd);
    client_map.erase(_client.m_client_fd);
    std::cerr << "[INFO] : Cleaning up from My Side" << std::endl;
}

void Check_Timeout()
{
    std::map<int, Client> &client_map = ValuesSingleton::getValuesSingleton()._clients_map;
    size_t timeout = getTime();

    std::map<www::fd_t, CGIs> &CGIsMap = CGIManagerSingleton::getCGIManagerSingleton().CGIsMap;
    std::map<www::fd_t, CGIs>::iterator ite = CGIsMap.begin();
    while (ite != CGIsMap.end())
    {
        if ((timeout - ite->second.m_lastUpdatedTime) >= TIMEOUT_CGI)
        {
            std::map<www::fd_t, CGIs>::iterator temp_ite = ite++;
            Client& _client = client_map[temp_ite->second.client_fd];

            if (temp_ite->second.Header_sent == false)
            {
                _client.m_CGItimeout = true;

                epoll_event event;
                event.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
                event.data.fd = _client.m_client_fd;
                epoll_ctl(ValuesSingleton::getValuesSingleton().epoll_fd, EPOLL_CTL_MOD, _client.m_client_fd, &event);
            }
            else
            {
                std::string theEnd = "0\r\n\r\n";
                send(temp_ite->second.client_fd, theEnd.c_str(), theEnd.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
                _clear(_client);
            }
        }
        else
            ++ite;
    }

    std::map<int, Client>::iterator it = client_map.begin();
    while (it != client_map.end())
    {
        if ((timeout - it->second.m_lastUpdatedTime) >= TIMEOUT_SIT)
        {
            std::map<int, Client>::iterator temp_it = it++;
            _clear(temp_it->second);
        }
        else
            ++it;
    }
}

void multiplexer(void)
{
    www::fd_t &epoll_fd = ValuesSingleton::getValuesSingleton().epoll_fd;
    std::map<int, Client> &client_map = ValuesSingleton::getValuesSingleton()._clients_map;
    std::map<www::fd_t, MyServerBlock>  &serverfd_map = ValuesSingleton::getValuesSingleton()._serverfd_map;
    std::vector<www::fd_t> &_CGIfds_vect = CGIManagerSingleton::getCGIManagerSingleton().CGIfds_vect;

    Check_Timeout(); // well It Does What the Name implies

    epoll_event events[MAX_EVENTS];
    int n = epoll_wait(epoll_fd, events, MAX_EVENTS, 10000);
    if (-1 == n)
    {
        if (errno == EINTR)
            return ;
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
            temp.m_lastUpdatedTime = getTime();
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

                std::map<int, Client>::iterator it = client_map.find(CGItohandle.client_fd);
                if (it == client_map.end())
                    continue;
                Client& _client = it->second;

                if (events[index].events & EPOLLIN)
                {
                    CGIManagerSingleton::ManagingCGI(CGItohandle, _client, events[index].data.fd);
                    continue;
                }
                if (events[index].events & EPOLLERR || events[index].events & EPOLLHUP)
                {
                    if (CGItohandle.Header_sent == true)
                    {
                        std::string theEnd = "0\r\n\r\n";
                        if (send(CGItohandle.client_fd, theEnd.c_str(), theEnd.size(), MSG_DONTWAIT | MSG_NOSIGNAL) == -1)
                            continue;
                        _clear(_client);
                    }
                    else 
                        _client.response_Error(502, true);
                }
                continue;
            }
            // CGI STUFF ^

            std::map<int, Client>::iterator it = client_map.find(events[index].data.fd);
            if (it == client_map.end())
                continue;

            if (events[index].events & EPOLLERR || events[index].events & EPOLLHUP)
            {
                std::cout << "[INFO] : (A Client Disconnected with an event other than EPOLLIN) fd Reserved = "<< events[index].data.fd << std::endl;
                _clear(it->second);
                continue;
            }
            else if (events[index].events & EPOLLIN || events[index].events & EPOLLOUT)
            {
                try
                {
                    Client &_client = it->second;
                    if (_client.readyto_send == false)
                    {
                        if (events[index].events & EPOLLOUT)
                        {
                            if (it->second.m_CGItimeout == true)
                                it->second.response_Error(504, true);
                        }

                        char buffer[ReadingSize];
                        memset(buffer, 0, sizeof(buffer));
                        ssize_t bytes_read = recv(_client.m_client_fd, buffer, sizeof(buffer), MSG_DONTWAIT);

                        if (-1 == bytes_read)
                            continue;
                        else if (0 == bytes_read)
                        {
                            std::cout << "[INFO] : (A Client Disconnected) fd Reserved = : "<< events[index].data.fd << std::endl;
                            _clear(_client);
                            continue;
                        }
                        else if (0 < bytes_read)
                        {
                            _client.m_lastUpdatedTime = getTime();
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
                catch (std::logic_error &e) {
                    throw e;
                }
                catch (std::runtime_error &e) {
                    std::cerr << www::RED << "[ERROR] : " << e.what() << www::RESET << std::endl;
                    continue;
                }
                catch (std::exception &e) {
                    std::cerr << www::YELLOW << "[DEBUG] : " << e.what() << www::RESET << std::endl;
                    continue;
                }
                catch (const int &i) {
                    switch (i) {
                        case CONTINUE: continue;
                        case END: { std::cout << "[INFO] : Closing the Connection to the Client by My side" << std::endl ; continue; }
                        default: continue;
                    }
                }
            }
        }
    }
}