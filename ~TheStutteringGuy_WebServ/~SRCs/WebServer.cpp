#include "WebServer.hpp"
#include <exception>
#include <iostream>
#include <vector>

const std::string www::Allowed_Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .-_~:/?#[]@!$&'()*+,;=%";
const std::string www::RED = "\033[31m";
const std::string www::GREEN = "\033[32m";
const std::string www::YELLOW = "\033[33m";
const std::string www::BLUE = "\033[34m";
const std::string www::MAGENTA = "\033[35m";
const std::string www::CYAN = "\033[36m";
const std::string www::RESET = "\033[0m";

bool www::SHUTDOWN = false;
void handle_sig(int) 
{
    std::cout << std::endl << www::YELLOW <<"[WARNING] : SIGINT Triggered" << www::RESET << std::endl; 
    www::SHUTDOWN = true; 
}

void Print_Infos(void)
{
    std::vector<MyServerBlock> &servers_blocks = ValuesSingleton::getValuesSingleton().servers_blocks;

    for (size_t index = 0; index < servers_blocks.size(); ++index)
    {
        std::cout << "╔═══════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║                  🚀 Server " << index << "                      ║" << std::endl;
        std::cout << "╠═══════════════════════════════════════════════════╣" << std::endl;
        std::cout << "║  📁 Root:        " << std::setw(33) << std::left << servers_blocks[index].m_root << "║" << std::endl;
        for (size_t i = 0; i < servers_blocks[index].IPs.size(); ++i)
            std::cout << "║  💾 IP/Port:     " << std::setw(33) << std::left << (servers_blocks[index].IPs[i] + "/" + to_string_c98(servers_blocks[index].Ports[i])) << "║" << std::endl;
        std::cout << "║  💾 Max Body:    " << std::setw(33) << std::left << servers_blocks[index].m_client_max_body_size << "║" << std::endl;
        std::cout << "╚═══════════════════════════════════════════════════╝" << std::endl << std::endl;
    } 
}

int API::Webserver(void)
{
    Print_generic_Rules();
    try
    {
        std::vector<MyServerBlock> &server_blocks = ValuesSingleton::getValuesSingleton().servers_blocks;
        std::map<www::fd_t, MyServerBlock> &serverfd_map = ValuesSingleton::getValuesSingleton()._serverfd_map;

        for (size_t index = 0; index < server_blocks.size(); ++index)
        {
            std::string &l_cache = server_blocks[index].m_cache;

            if (access(l_cache.c_str(), F_OK | R_OK | W_OK) != 0)
                throw std::logic_error("Please Follow The Rules => (Cache/Uplaod) one");
        }

        www::fd_t epoll_fd = epoll_create1(0);
        if (-1 == epoll_fd)
            throw std::logic_error("epoll_create() " + static_cast<std::string>(strerror(errno)));
        ValuesSingleton::getValuesSingleton().epoll_fd = epoll_fd;

        for (size_t index = 0; index < server_blocks.size(); ++index)
        {
            std::vector<std::string> vect = server_blocks[index].m_directives["listen"];
            for (std::vector<std::string>::iterator it = vect.begin(); it != vect.end(); ++it)
            {
                std::string addr;
                std::string port;

                std::string value = *it;
                
                size_t pos = value.find(":");
                if (pos != std::string::npos)
                {
                    addr = value.substr(0, pos);
                    port = value.substr(pos + static_cast<std::string>(":").size());
                }
                else
                    port = value;

                www::fd_t socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
                if (-1 == socket_fd)
                    throw std::logic_error("socket() " + static_cast<std::string>(strerror(errno)));
                socklen_t optival = 1;
                if (-1 == setsockopt(socket_fd, SOL_SOCKET ,SO_REUSEADDR | SO_REUSEPORT, &optival, sizeof(optival)))
                    throw std::logic_error("setsocketopt() " + static_cast<std::string>(strerror(errno)));

                const char* addr_cstr;
                addrinfo hints, *result;
                memset(&hints, 0, sizeof(addrinfo));
                hints.ai_family = AF_INET;
                hints.ai_socktype = SOCK_STREAM;
                hints.ai_protocol = 0;
                if (addr.empty())
                {
                    hints.ai_flags = AI_PASSIVE;
                    addr_cstr = NULL;
                }
                else
                {
                    hints.ai_flags = 0;
                    addr_cstr = addr.c_str();
                }

                int status = getaddrinfo(addr_cstr, port.c_str(), &hints, &result);
                if (status != 0)
                    throw std::logic_error("Error : " + static_cast<std::string>(gai_strerror(status)));
                ValuesSingleton::getValuesSingleton().addrinfo_vect.push_back(result);
            
                if (-1 == bind(socket_fd, result->ai_addr, result->ai_addrlen))
                    throw std::logic_error("bind() " + static_cast<std::string>(strerror(errno)));
                if (-1 == listen(socket_fd, 1024))
                    throw std::logic_error("listen() " + static_cast<std::string>(strerror(errno)));
                
                sockaddr_in *sockaddr = reinterpret_cast<sockaddr_in *>(result->ai_addr);

                char ipStr[INET_ADDRSTRLEN];
                server_blocks[index].IPs.push_back(inet_ntop(result->ai_family, &sockaddr->sin_addr, ipStr, sizeof(ipStr)));
                server_blocks[index].Ports.push_back(ntohs(sockaddr->sin_port));
                
                epoll_event event;
                event.events = EPOLLIN;
                event.data.fd = socket_fd;
                if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event))
                    throw std::logic_error("epoll_ctl() " + static_cast<std::string>(strerror(errno)));
                
                serverfd_map[socket_fd] = server_blocks[index];
            }
        }
        Print_Infos();
        signal(SIGINT, handle_sig);
        signal(SIGPIPE, SIG_IGN);
        while (www::SHUTDOWN == false)
        {
            while (waitpid(-1, NULL, WNOHANG) > 0) { ;}
            try {
                multiplexer();
            }
            catch(const std::logic_error &e) {
                throw e;
            }
            catch(const std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
            catch(...) {
                std::cerr << "An unknown exception was caught in the loop!" << std::endl;
            }
        }
    }
    catch (std::logic_error &e) {
        std::cerr << www::MAGENTA << "[FATAL] : " << e.what() << www::RESET << std::endl;
    }
    catch (...) {
        std::cerr << "An unknown exception was caught in the main loop!" << std::endl;
    }

    class ValuesSingleton& cleanup = ValuesSingleton::getValuesSingleton();
    for (size_t index = 0; index < cleanup.addrinfo_vect.size(); ++index)
        freeaddrinfo(cleanup.addrinfo_vect[index]);

    class CGIManagerSingleton& CGImanager = CGIManagerSingleton::getCGIManagerSingleton();
    for (std::vector<www::fd_t>::iterator it = CGImanager.CGIfds_vect.begin(); it != CGImanager.CGIfds_vect.end(); ++it)
        CGImanager.CGIeraseFrom_Map(*it);
    return 0;
}