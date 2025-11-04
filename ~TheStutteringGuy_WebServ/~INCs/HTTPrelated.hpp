#pragma once
#include "../~INCs/'Standards'.hpp"

#include "Namespaces.hpp"
#include "HTTPconst.hpp"
#include <algorithm>
#include <cstddef>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ServerBlock :

struct MyLocationBlock
{
    MyLocationBlock() : autoindex(false), is_Red(false), is_CGI(false) {};

    std::map<std::string, std::vector<std::string> > directives;

    std::string root;
    std::string index;
    bool        autoindex;

    std::vector<std::string>    allowed_methods;

    bool                        is_Red;
    std::map<int, std::string>  redirection;

    bool                                is_CGI;
    std::map<std::string, std::string>  cgi_infos;

    bool                is_Upload;
    std::string         uplaod_path;
    std::string         actual_URI;
};

struct MyServerBlock
{
    MyServerBlock() : m_client_max_body_size(524288000) {};

    std::map<std::string, std::vector<std::string> > m_directives;
    std::map<std::string, MyLocationBlock> m_locationBlocks;

    std::vector<std::string>    IPs;
    std::vector<int>            Ports;

    std::string m_root;
    std::string m_cache;
    // std::string m_upload;

    size_t                      m_client_max_body_size;
    std::map<int, std::string>  m_error_pages;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RESPONSE / REQUEST / CLIENT :

struct Response // this is more like struct HEADER
{
    std::string     m_HTTP_version;
    unsigned int    m_status_code;

    bool            m_content_needed;

    std::string     m_content_type;
    int             m_content_length;

    Response() : m_HTTP_version("HTTP/1.0") {};
    ~Response() {};
    Response(std::string HTTP_version, unsigned int status_code, bool content_needed, std::string content_type, int content_length)
        : m_HTTP_version(HTTP_version), m_status_code(status_code), m_content_needed(content_needed), m_content_type(content_type), m_content_length(content_length) {}
};

struct Request
{
    std::string m_method;

    std::string m_URI;
    std::string m_queryString;

    std::string m_version;
    std::map<std::string, std::vector<std::string> > m_headers;

    std::string m_body;

    // Added by Ahmed
    std::map<std::string, std::string> m_cookie;
};

inline std::string uid_generator(void)
{
    struct timespec time;

    clock_gettime(CLOCK_REALTIME, &time);

    size_t value;

    value = time.tv_sec * 1000000000ULL + time.tv_nsec;

    std::stringstream ss;

    ss << value;
    return ss.str();
}

class Client
{
public:
    www::fd_t       m_client_fd;
    MyServerBlock   m_Myserver;

    bool header_done;
    bool need_body;
    bool m_isChunked;
    bool handling_request;
    bool readyto_send;

    std::string m_request_buffer;
    std::string m_headers_buffer;
    std::string m_body_buffer;

private:
    Request     m_request;
    std::string m_response_buffer;

private:
    bool m_body_asFile_init;
    bool m_response_asFile_init;

public:
    std::fstream    *m_body_asFile;
    std::string     m_body_asFile_path;

private:
    bool            m_response_is_aFile;
    std::fstream    *m_response_asFile;
    std::string     m_response_asFile_path;

public:
    size_t      m_lastUpdatedTime;
    www::fd_t   m_CGIfd;
    bool        m_CGItimeout;

private: // ahmed
    std::string     session_id;
    session         session_data;
    bool            m_session_needs_set_cookie;

public:
    Client() : m_client_fd(-1), header_done(false), need_body(true), m_isChunked(false), handling_request(false), readyto_send(false), m_body_asFile_init(false), m_response_asFile_init(false), m_body_asFile(NULL), m_response_is_aFile(false), m_response_asFile(NULL), m_lastUpdatedTime(0), m_CGIfd(-1), m_CGItimeout(false) {};

    Client(www::fd_t client_fd) : m_client_fd(client_fd), header_done(false), need_body(true), m_isChunked(false), handling_request(false), readyto_send(false), m_body_asFile_init(false), m_response_asFile_init(false), m_body_asFile(NULL), m_response_is_aFile(false), m_response_asFile(NULL), m_lastUpdatedTime(0), m_CGIfd(-1), m_CGItimeout(false) {};
    ~Client()
    {
        if (m_body_asFile != NULL)
        {
            m_body_asFile->close();
            delete m_body_asFile;
            std::remove(m_body_asFile_path.c_str());
        }
        if (m_response_asFile != NULL)
        {
            m_response_asFile->close();
            delete m_response_asFile;
        }
    };
    Client(const Client &other) : m_body_asFile(NULL), m_response_asFile(NULL) { *this = other; };

public:
    Client &operator=(const Client &other)
    {
        if (this == &other)
            return *this;

        if (this->m_body_asFile != NULL)
            delete m_body_asFile;
        if (this->m_response_asFile != NULL)
            delete m_response_asFile;

        this->m_client_fd = other.m_client_fd;
        this->m_Myserver = other.m_Myserver;
        this->header_done = other.header_done;
        this->need_body = other.need_body;
        this->m_isChunked = other.m_isChunked;
        this->handling_request = other.handling_request;
        this->readyto_send = other.readyto_send;
        this->m_request_buffer = other.m_request_buffer;
        this->m_headers_buffer = other.m_headers_buffer;
        this->m_body_buffer = other.m_body_buffer;
        this->m_request = other.m_request;
        this->m_response_buffer = other.m_response_buffer;
        this->m_lastUpdatedTime = other.m_lastUpdatedTime;
        this->m_CGIfd = other.m_CGIfd;
        this->m_CGItimeout = other.m_CGItimeout;    

        this->m_body_asFile = NULL;
        this->m_response_asFile = NULL;

        this->m_body_asFile_init = false;
        this->m_response_asFile_init = false;
        this->m_body_asFile_path = std::string();
        this->m_response_is_aFile = false;
        this->m_response_asFile_path = std::string();
        return *this;
    }

public:
    void initialize_body_asFile(void)
    {
        if (this->m_body_asFile_init == false)
        {
            this->m_body_asFile = new std::fstream();

            std::string l_string = m_Myserver.m_cache + uid_generator();
            this->m_body_asFile_path = l_string;
            m_body_asFile->open(l_string.c_str(), std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
            if (!m_body_asFile->is_open())
                throw std::runtime_error("m_body_asFile.isopem()");
            this->m_body_asFile_init = true;
        }
    }
    void initialize_response_asFile(const std::string &filename)
    {
        if (m_response_asFile_init == false)
        {
            this->m_response_asFile = new std::fstream();

            this->m_response_asFile_path = filename;
            this->m_response_asFile->open(filename.c_str(), std::ios::in | std::ios::binary);
            if (!m_response_asFile->is_open())
                throw std::runtime_error("m_body_asFile.isopem()");
            m_response_asFile_init = true;
        }
    }

public:
    bool headerEnd_Checker(void);
    void headers_parser(void);
    void URI_parser(void);
    void headers_implication(void);
    bool body_checker(void);

public:
    void response_Error(const unsigned int &error_code, bool Content_needed);
    void response_html_ready(const std::string &string_);

public:
    void handle_Request(void);
    void handle_Response(void);

private:
    void handle_GET(MyLocationBlock &p_locationBlock, std::string &actual_URI);
    void handle_POST(MyLocationBlock &p_locationBlock);
    void handle_DELETE(MyLocationBlock &p_locationBlock, const std::string &actual_URI);
    void response_Get(const std::string &File);
    void response_justAstatus(const unsigned int &status_code);

private:
    pid_t Handle_CGI(const std::string bin, const std::string actual_URI, www::fd_t *sv, const std::string &body_File);
    // Added by Ahmed
private:
    std::string trim_whitespaces(const std::string &str);
    void parse_cookie();
    std::string serialize_cookies() const;
    // for sessions
    void handle_session_logic();
    std::string get_session_cookie_header();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Singletons :

class ValuesSingleton
{
private:
    ValuesSingleton() {};
    // ValuesSingleton(const ValuesSingleton& other);
    // ValuesSingleton& operator=(const ValuesSingleton& other);

public:
    std::vector<addrinfo *>     addrinfo_vect;
    www::fd_t                   epoll_fd;
    std::vector<MyServerBlock>  servers_blocks;
    std::vector<www::fd_t>      open_fds;

    std::map<www::fd_t, MyServerBlock>  _serverfd_map;
    std::map<www::fd_t, Client>         _clients_map;

    struct HTTPstatus_phrase        m_HTTPstatus_phrase;
    struct FileContent_type         m_FileContent_type;
    struct Reverse_FileContent_type m_Reverse_FileContent_type;

    static ValuesSingleton &getValuesSingleton()
    {
        static ValuesSingleton Singleton;
        return Singleton;
    }
};

struct CGIs
{
    www::fd_t   client_fd;
    pid_t       CGIpid;
    bool        Header_sent;
    size_t      m_lastUpdatedTime;

    std::string m_recv_buffer;
    std::string m_headers_buffer;
    std::string m_body_buffer;
};

class CGIManagerSingleton
{
public:
    std::vector<www::fd_t>      CGIfds_vect;
    std::map<www::fd_t, CGIs>   CGIsMap;

private:
    CGIManagerSingleton() {};
    // CGIManagerSingleton(const CGIManagerSingleton& other);
    // CGIManagerSingleton& operator=(const CGIManagerSingleton& other);

public:
    static CGIManagerSingleton &getCGIManagerSingleton()
    {
        static CGIManagerSingleton Singleton;
        return Singleton;
    }

    void CGIeraseFrom_Map(www::fd_t &fd)
    {
        std::map<www::fd_t, CGIs>::iterator it = this->CGIsMap.find(fd);
        if (it == this->CGIsMap.end())
            return;

        epoll_ctl(ValuesSingleton::getValuesSingleton().epoll_fd, EPOLL_CTL_DEL, fd, NULL);
        kill((it->second).CGIpid, SIGKILL);

        this->CGIsMap.erase(it);
        close(fd);
    }
    void CGIeraseFrom_Vect(www::fd_t &fd)
    {
        std::vector<www::fd_t>::iterator it;
        it = std::find(this->CGIfds_vect.begin(), this->CGIfds_vect.end(), fd);
        if (this->CGIfds_vect.end() != it)
            this->CGIfds_vect.erase(it);
    }
    void CGIerase(www::fd_t &fd)
    {
        this->CGIeraseFrom_Vect(fd);
        this->CGIeraseFrom_Map(fd);
    }

    static void ManagingCGI(CGIs &CGItohandle, Client &_client, www::fd_t &CGIfd);
};

void multiplexer(void);

// ahmed
// session class
class sessionManager
{
private:
    std::map<std::string, session> m_session;
    time_t timeout_sec;

    sessionManager() : timeout_sec(3600) {};
    ~sessionManager() {};

public:
    static sessionManager &getSession()
    {
        static sessionManager instance;
        return (instance);
    };

    time_t getTimeout() const
    {
        return (timeout_sec);
    };

public:
    std::string generate_session_id();
    std::string create_session(const std::string &user_id = "");
    bool get_session(const std::string &session_id, session &out_session);
    void cleanup_session();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions :

template <typename T>
static std::string to_string_c98(T value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

inline size_t getTime()
{
    timeval time;

    gettimeofday(&time, NULL);

    size_t value_usec = static_cast<size_t>(time.tv_sec) * 1000000 + time.tv_usec;

    return value_usec;
}

inline std::string readFile(const std::string &filename)
{
    std::fstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("file.open()" + static_cast<std::string>(strerror(errno)));

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer(size, '\0');
    file.read(&buffer[0], size);
    return buffer;
}

inline const std::vector<std::string> *find_Value_inMap(const std::map<std::string, std::vector<std::string> > &map, const std::string &key)
{
    std::map<std::string, std::vector<std::string> >::const_iterator it = map.find(key);
    if (it != map.end())
        return &(it->second);
    return NULL;
}

// Response Related :

inline static std::string get_http_date()
{
    char buf[128];
    time_t t = time(NULL);
    tm *tm = gmtime(&t);
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", tm);
    return std::string(buf);
}

inline std::string headers_Creator(struct Response response)
{
    std::string end = "\r\n";
    std::string eoh = "\r\n\r\n";
    std::string s = " ";

    std::string l_status_code = to_string_c98(response.m_status_code);
    std::string l_phrase = s + ValuesSingleton::getValuesSingleton().m_HTTPstatus_phrase.getStatusPhrase(l_status_code);

    std::string Response = response.m_HTTP_version + s + l_status_code + s + l_phrase + end;
    Response += "Date: " + get_http_date() + end;
    Response += "Connection: close" + end;
    if (true == response.m_content_needed)
    {
        Response += "Content-Type: " + response.m_content_type + end;
        Response += "Content-Length: " + to_string_c98(response.m_content_length);
    }
    Response += eoh;

    return Response;
}

inline std::string headers_Creator(struct Response response, int)
{
    std::string end = "\r\n";
    std::string s = " ";

    std::string l_status_code = to_string_c98(response.m_status_code);
    std::string l_phrase = s + ValuesSingleton::getValuesSingleton().m_HTTPstatus_phrase.getStatusPhrase(l_status_code);

    std::string Response = response.m_HTTP_version + s + l_status_code + s + l_phrase + end;
    Response += "Date: " + get_http_date() + end;
    Response += "Connection: close" + end;
    if (true == response.m_content_needed)
    {
        Response += "Content-Type: " + response.m_content_type + end;
        Response += "Content-Length: " + to_string_c98(response.m_content_length);
    }
    return Response;
}

// ahmed headers creator overload function for sessions
inline std::string headers_Creator(struct Response response, std::string &set_cookie_header_to_append, bool is_header_block)
{
    std::string end = "\r\n";
    std::string eoh = "\r\n\r\n";
    std::string s = " ";

    std::string l_status_code = to_string_c98(response.m_status_code);
    std::string l_phrase = s + ValuesSingleton::getValuesSingleton().m_HTTPstatus_phrase.getStatusPhrase(l_status_code);

    std::string Response = response.m_HTTP_version + s + l_status_code + s + l_phrase + end;
    Response += "Date: " + get_http_date() + end;
    Response += "Connection: close" + end;
    if (!set_cookie_header_to_append.empty())
        Response += set_cookie_header_to_append;

    Response += "Content-Type: " + response.m_content_type + end;
    Response += "Content-Length: " + to_string_c98(response.m_content_length) + end;

    if (is_header_block)
        Response += eoh;

    return (Response);
};

// inline std::string getFullPath(const std::string& relative_path)
// {
//     char resolved_path[PATH_MAX];
//     if (realpath(relative_path.c_str(), resolved_path) != NULL)
//         return std::string(resolved_path);
//     return "";
// }