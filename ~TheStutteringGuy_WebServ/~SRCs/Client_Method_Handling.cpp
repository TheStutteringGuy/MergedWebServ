#include "WebServer.hpp"

static std::string getParentPath(const std::string& filepath)
{
    std::string parent_path;
    size_t pos;

    pos = filepath.find_last_of('/');
    parent_path = filepath.substr(0, pos + 1);

    return parent_path;
}

void Client::handle_DELETE(MyLocationBlock &p_locationBlock, const std::string& actual_URI)
{
    struct stat test;

    (void)p_locationBlock;
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

void Client::handle_GET(MyLocationBlock &p_locationBlock, std::string& actual_URI)
{
    struct stat test;

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
    (void)p_locationBlock;

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
    std::string uri(this->m_request.m_URI);

    while (true)
    {
        // Check if the current URI matches any location block
        if (this->m_Myserver.m_locationBlocks.find(uri) != this->m_Myserver.m_locationBlocks.end())
        {
            tmp_location = uri;
            break;
        }

        if (uri == "/" || uri.empty())
            this->response_Error(404, true);

        size_t pos = uri.find_last_of("/");
        if (pos == 0)
            uri = "/";
        else if (pos != std::string::npos)
            uri = uri.substr(0, pos);
    }

    MyLocationBlock &tmp_locationBlock = this->m_Myserver.m_locationBlocks[tmp_location];
    std::vector<std::string> &tmp_allowedMethods = tmp_locationBlock.allowed_methods;
    if (tmp_locationBlock.root.empty())
        tmp_locationBlock.root = this->m_Myserver.m_root;

    std::string actual_URI(this->m_request.m_URI);
    size_t pos = actual_URI.find(tmp_location);
    actual_URI.replace(pos, tmp_location.size(), tmp_locationBlock.root);

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

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

    bool get(false), post(false);

    if (this->m_request.m_method == "GET")
    {
        if (std::find(tmp_allowedMethods.begin(), tmp_allowedMethods.end(), "GET") != tmp_allowedMethods.end())
            get = true;
        else
            this->response_Error(405, true);
    }    
    else if (this->m_request.m_method == "POST")
    {
        if (std::find(tmp_allowedMethods.begin(), tmp_allowedMethods.end(), "POST") != tmp_allowedMethods.end())
            post = true;
        else
            this->response_Error(405, true);
    }

    while(tmp_locationBlock.is_CGI == true)
    {
        std::string bin;
        size_t pos = actual_URI.find_last_of(".");

        if (pos == std::string::npos)
            break;

        std::string extension = actual_URI.substr(pos);
        if (tmp_locationBlock.cgi_infos.find(extension) == tmp_locationBlock.cgi_infos.end())
            break;
        else
            bin = tmp_locationBlock.cgi_infos[extension];

        www::fd_t sv[2];
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1)
        {
            std::cerr << "socketpair() " + static_cast<std::string>(strerror(errno)) << std::endl;
            this->response_Error(500, true);
        }

        CGIManagerSingleton &CGImanager = CGIManagerSingleton::getCGIManagerSingleton();

        CGIs& obj = CGImanager.CGIsMap[sv[0]];

        obj.client_fd = this->m_client_fd;
        obj.timeout = getTime();

        obj.CGIpid = this->Handle_CGI(bin, actual_URI, sv);
        close (sv[1]);

        CGImanager._CGIfds_vect.push_back(sv[0]);

        epoll_event event;
        event.events = EPOLLIN | EPOLLHUP | EPOLLERR;
        event.data.fd = sv[0];
        if (epoll_ctl(ValuesSingleton::getValuesSingleton().epoll_fd, EPOLL_CTL_ADD, sv[0], &event) == -1)
            throw std::logic_error("epoll_ctl() " + static_cast<std::string>(strerror(errno)));
    
        throw CONTINUE;
    }

    if (get == true)
            this->handle_GET(tmp_locationBlock, actual_URI);   
    else if (post == true)
            this->handle_POST(tmp_locationBlock);  
    else if (this->m_request.m_method == "DELETE")
    {
        if (std::find(tmp_allowedMethods.begin(), tmp_allowedMethods.end(), "DELETE") != tmp_allowedMethods.end())
            this->handle_DELETE(tmp_locationBlock, actual_URI);
        else
            this->response_Error(405, true);
    }
}