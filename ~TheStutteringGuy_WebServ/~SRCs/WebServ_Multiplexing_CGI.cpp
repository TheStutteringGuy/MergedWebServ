#include "WebServer.hpp"
#include <sys/socket.h>

#define CLEAR -1;

static bool manage_Headers(CGIs& CGItohandle)
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

void Update_time(CGIs& CGItohandle, Client& _client)
{
    CGItohandle.m_lastUpdatedTime = getTime();
    _client.m_lastUpdatedTime = getTime();
}

void CGIManagerSingleton::ManagingCGI(CGIs& CGItohandle, Client& _client, www::fd_t& CGIfd)
{
    try
    {
        char buffer[ReadingSize];
        memset(buffer, 0, sizeof(buffer));
        ssize_t read_bytes = recv(CGIfd, buffer, sizeof(buffer), MSG_DONTWAIT | MSG_NOSIGNAL);
        Update_time(CGItohandle, _client);

        if (read_bytes == -1)
            return ;

        if (CGItohandle.Header_sent == false)
        {
            if (read_bytes == 0)
            {
                _client.response_Error(502, true);
                return ;
            }

            if (read_bytes > 0)
            {
                CGItohandle.m_recv_buffer.append(buffer, read_bytes);

                bool Headers_retrieved = manage_Headers(CGItohandle);
                if (Headers_retrieved == false)
                    return;

                std::string Header = headers_Creator(Response("HTTP/1.1", 200, false, "", 0), 0);
                Header += "Transfer-Encoding: chunked\r\n";
                
                if (send(CGItohandle.client_fd, Header.c_str(), Header.size(), MSG_DONTWAIT | MSG_NOSIGNAL) == -1)
                    return ;
                Update_time(CGItohandle, _client);
                if (send(CGItohandle.client_fd, CGItohandle.m_headers_buffer.c_str(), CGItohandle.m_headers_buffer.size(), MSG_DONTWAIT | MSG_NOSIGNAL) == -1)
                    return ;
                Update_time(CGItohandle, _client);

                CGItohandle.Header_sent = true;
                if (!CGItohandle.m_body_buffer.empty())
                {
                    std::stringstream ss;
                    ss << std::hex << CGItohandle.m_body_buffer.size();
                    std::string size = ss.str();

                    if (send(CGItohandle.client_fd, size.c_str(), size.size(), MSG_DONTWAIT | MSG_NOSIGNAL) == -1)
                        return ;
                    Update_time(CGItohandle, _client);

                    if (send(CGItohandle.client_fd, "\r\n", 2, MSG_DONTWAIT | MSG_NOSIGNAL) == -1)
                        return ;
                    Update_time(CGItohandle, _client);

                    if (send(CGItohandle.client_fd, CGItohandle.m_body_buffer.c_str(), CGItohandle.m_body_buffer.size(), MSG_DONTWAIT | MSG_NOSIGNAL) == -1)
                        return ;
                    Update_time(CGItohandle, _client);

                    if (send(CGItohandle.client_fd, "\r\n", 2, MSG_DONTWAIT | MSG_NOSIGNAL) == -1)
                        return ;
                    Update_time(CGItohandle, _client);

                }
            }
        }
        else 
        {
            if (read_bytes == 0)
            {
                std::string theEnd = "0\r\n\r\n";
                if (send(CGItohandle.client_fd, theEnd.c_str(), theEnd.size(), MSG_DONTWAIT | MSG_NOSIGNAL) == -1)
                {
                    std::cerr <<  "send() Failed To Send The Last Chunk" << std::endl;
                    return;
                }
                throw CLEAR;
            }
            if (read_bytes > 0)
            {
                std::stringstream ss;
                ss << std::hex << read_bytes;
                std::string size = ss.str();

                if (send(CGItohandle.client_fd, size.c_str(), size.size(), MSG_DONTWAIT | MSG_NOSIGNAL) == -1)
                    return;
                Update_time(CGItohandle, _client);
                if (send(CGItohandle.client_fd, "\r\n", 2, MSG_DONTWAIT | MSG_NOSIGNAL) == -1)
                    return;
                Update_time(CGItohandle, _client);
                if (send(CGItohandle.client_fd, buffer, read_bytes, MSG_DONTWAIT | MSG_NOSIGNAL) == -1)
                    return;
                Update_time(CGItohandle, _client);
                if (send(CGItohandle.client_fd, "\r\n", 2, MSG_DONTWAIT | MSG_NOSIGNAL) == -1)
                    return;
                Update_time(CGItohandle, _client);
            }
        }
    }
    catch (int &checker) {
        if (checker == -1)
            _clear(_client);
        return ;
    }
}