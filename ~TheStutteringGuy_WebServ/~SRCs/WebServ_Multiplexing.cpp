#include "WebServer.hpp"
#include <cerrno>

#define TIMEOUT_SEC 30000000

void _clear(Client &_client)
{
    std::map<int, Client> &client_map = ValuesSingleton::getValuesSingleton()._clients_map;

    close(_client.m_client_fd);
    epoll_ctl(ValuesSingleton::getValuesSingleton().epoll_fd, EPOLL_CTL_DEL, _client.m_client_fd, NULL);
    client_map.erase(_client.m_client_fd);
}

void multiplexer(void)
{
    www::fd_t &epoll_fd = ValuesSingleton::getValuesSingleton().epoll_fd;
    std::map<int, Client> &client_map = ValuesSingleton::getValuesSingleton()._clients_map;
    std::map<www::fd_t, MyServerBlock>  &serverfd_map = ValuesSingleton::getValuesSingleton()._serverfd_map;
    std::vector<www::fd_t> &_CGIfds_map = CGIManagerSingleton::getCGIManagerSingleton()._CGIfds_map;

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
                continue;
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
            if (std::find(_CGIfds_map.begin(), _CGIfds_map.end(), events[index].data.fd) != _CGIfds_map.end())
            {
                // handleCGI_LOGIC;
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
                    static bool checker;

                    if (checker == false)
                    {
                        std::string &l_cache = (_client.m_Myserver.m_cache = _client.m_Myserver.m_root);
                        l_cache += static_cast<std::string>(cache);
                        std::string l_upload = (_client.m_Myserver.m_root + static_cast<std::string>(upload_dir));
                        if (access(l_cache.c_str(), F_OK | R_OK | W_OK) != 0 || access(l_upload.c_str(), F_OK | R_OK | W_OK) != 0 )
                            _client.response_Error(500, true);
                        checker = true;
                    }
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
                throw;
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