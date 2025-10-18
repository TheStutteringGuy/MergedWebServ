#include "WebServer.hpp"
#include <algorithm>
#include <unistd.h>

static std::string getParentPath(const std::string& filepath)
{
    std::string parent_path;
    size_t pos;

    pos = filepath.find_last_of('/');
    parent_path = filepath.substr(0, pos + 1);

    return parent_path;
}

void Client::handle_DELETE(MyLocationBlock &p_locationBlock)
{
    struct stat test;

    (void)p_locationBlock;

    std::string actual_URI(this->m_request.m_URI);
    actual_URI.erase(0, 1);
    actual_URI.insert(0, p_locationBlock.root);

    if (access(actual_URI.c_str(), F_OK) != 0)
        this->response_Error(404, true);

    stat(actual_URI.c_str(), &test);
    if (S_ISDIR(test.st_mode))
        this->response_Error(403, true);

    std::string parent_path = getParentPath(actual_URI);
    if (access(parent_path.c_str(), W_OK) != 0)
        this->response_Error(403, true);

    std::remove(actual_URI.c_str());
    this->response_justAstatus(204);
}

void Client::handle_GET(MyLocationBlock &p_locationBlock)
{
    struct stat test;
    
    std::string actual_URI(this->m_request.m_URI);
    actual_URI.erase(0, 1);
    actual_URI.insert(0, p_locationBlock.root);

    if (p_locationBlock.is_CGI == true)
    {
        // Handle_CGI;
    }

    if (access(actual_URI.c_str(), F_OK) != 0)
        this->response_Error(404, true);
    
    stat(actual_URI.c_str(), &test);
    if (S_ISDIR(test.st_mode))
    {
        if (!p_locationBlock.index.empty())
        {
            if (access((p_locationBlock.root + p_locationBlock.index).c_str(), F_OK | R_OK) != 0)
                this->response_Error(404, true);
            this->response_Get((p_locationBlock.root + p_locationBlock.index));
        }
        else if (p_locationBlock.autoindex == true)
        {
            // listing the directory
            if (this->m_request.m_URI[this->m_request.m_URI.size() - 1] != '/')
            {
                this->m_request.m_URI.append("/");
                actual_URI.append("/");
            }
    
            if (access(actual_URI.c_str(), R_OK | X_OK) != 0)
                this->response_Error(403, true);
    
            DIR* dir_ptr;
            struct dirent *entry;
    
            dir_ptr = opendir(actual_URI.c_str());
            if (NULL == dir_ptr)
                this->response_Error(503, true);
    
            std::string to_send;
            std::string s = " ";
            std::string end = "\n";
    
            to_send = generic_index_page(this->m_request.m_URI);
    
            size_t pos = to_send.find("INSERT HERE");
            if (pos != std::string::npos)
                to_send.erase(pos, static_cast<std::string>("INSERT HERE").size());
            while ((entry = readdir(dir_ptr)) != NULL)
            {
                if (entry->d_name[0] != '.')
                {
                    if (entry->d_type == DT_DIR)
                    {
                        std::stringstream ss;
                        ss << "            <li><a href=\"" << this->m_request.m_URI + entry->d_name << "/\">" << entry->d_name << '/' << "</a></li>" << end;
                        to_send.insert(pos, ss.str());
                        pos += ss.str().size();
                    }
                    else if (entry->d_type == DT_REG)
                    {
                        std::stringstream ss;
                        ss << "            <li><a href=\"" << this->m_request.m_URI + entry->d_name << "\" class=\"file-link\">" << entry->d_name << "</a></li>" << end;
                        to_send.insert(pos, ss.str());
                        pos += ss.str().size();
                    }
                }
            }
            closedir(dir_ptr);
            this->response_html_ready(to_send);
        }
        else
            this->response_Error(500, true);
    }
    else
    {
        if (access(actual_URI.c_str(), R_OK) != 0)
            this->response_Error(403, true);
        
        this->response_Get(actual_URI);
    }
}

void Client::handle_POST(MyLocationBlock &p_locationBlock)
{
    if (p_locationBlock.is_CGI == true)
    {
        std::string actual_URI(this->m_request.m_URI);
        actual_URI.erase(0, 1);
        actual_URI.insert(0, p_locationBlock.root);
        
        // Handle_CGI;
    }

    const std::vector<std::string> *content_type = find_Value_inMap(this->m_request.m_headers, "Content-Type");

    std::string media_type;
    if (NULL == content_type)
        media_type = "";
    else
    {
        media_type = ValuesSingleton::getValuesSingleton().m_Reverse_FileContent_type.getContent_type((*content_type)[0]);
    }

    std::string uid = uid_generator();
    std::string filename = uid + media_type;
    std::string upload_path = static_cast<std::string>(this->m_Myserver.m_root) + static_cast<std::string>(upload_dir);
    std::string final_path = upload_path + filename;

    if (std::rename(this->m_body_asFile_path.c_str(), final_path.c_str()) != 0)
    {
        this->response_Error(500, true);
        throw std::runtime_error("std::rename()" + static_cast<std::string>(strerror(errno)));
    }

    std::string Headers = headers_Creator(Response("HTTP/1.0", 201, false, std::string(), 0), 0);
    std::string Locattion_Header = "Location: " + static_cast<std::string>(upload_dir) + filename + "\r\n";

    Headers += Locattion_Header + "\r\n";

    this->m_response_buffer = Headers;
    this->readyto_send = true;

    epoll_event event;
    event.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
    event.data.fd = this->m_client_fd;
    epoll_ctl(ValuesSingleton::getValuesSingleton().epoll_fd, EPOLL_CTL_MOD, this->m_client_fd, &event);
    throw CONTINUE;
}

void Client::handle_Request(void)
{
    this->handling_request = true;
    std::string tmp_location;

    while (True)
    {
        if (this->m_Myserver.m_locationBlocks.find(this->m_request.m_URI) != this->m_Myserver.m_locationBlocks.end())
        {
            tmp_location = this->m_request.m_URI;
            break;
        }

        std::string tmp;
        size_t pos = this->m_request.m_URI.find_last_of("/");
        if (pos != std::string::npos)
            tmp = this->m_request.m_URI.substr(0, pos);

        if (tmp.empty()) { tmp = "/"; }
        if (this->m_Myserver.m_locationBlocks.find(tmp) != this->m_Myserver.m_locationBlocks.end())
        {
            tmp_location = tmp;
            break;
        }

        this->response_Error(404, true);
    }

    MyLocationBlock &tmp_locationBlock = this->m_Myserver.m_locationBlocks[tmp_location];
    
    if (tmp_locationBlock.is_Red == true)
    {
        std::map<int, std::string>::iterator first = tmp_locationBlock.redirection.begin();

        std::string Headers = headers_Creator(Response("HTTP/1.0", first->first, false, std::string(), 0), 0);
        std::string Locattion_Header = "Location: " + first->second + "\r\n";

        Headers += Locattion_Header + "\r\n";

        this->m_response_buffer = Headers;
        this->readyto_send = true;

        epoll_event event;
        event.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
        event.data.fd = this->m_client_fd;
        epoll_ctl(ValuesSingleton::getValuesSingleton().epoll_fd, EPOLL_CTL_MOD, this->m_client_fd, &event);
        throw CONTINUE;
    }

    std::vector<std::string> &tmp_allowedMethods = tmp_locationBlock.allowed_methods;
    if (tmp_locationBlock.root.empty())
        tmp_locationBlock.root = this->m_Myserver.m_root;

    if (this->m_request.m_method == "GET")
    {
        if (std::find(tmp_allowedMethods.begin(), tmp_allowedMethods.end(), "GET") != tmp_allowedMethods.end())
            this->handle_GET(tmp_locationBlock);
        else
            this->response_Error(405, true);
    }    
    else if (this->m_request.m_method == "POST")
    {
        if (std::find(tmp_allowedMethods.begin(), tmp_allowedMethods.end(), "POST") != tmp_allowedMethods.end())
            this->handle_POST(tmp_locationBlock);
        else
            this->response_Error(405, true);
    }    
    else if (this->m_request.m_method == "DELETE")
    {
        if (std::find(tmp_allowedMethods.begin(), tmp_allowedMethods.end(), "DELETE") != tmp_allowedMethods.end())
            this->handle_DELETE(tmp_locationBlock);
        else
            this->response_Error(405, true);
    }
}