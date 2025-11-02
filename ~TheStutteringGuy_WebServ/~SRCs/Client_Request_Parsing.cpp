#include "WebServer.hpp"
#include <iosfwd>
#include <unistd.h>

bool Client::headerEnd_Checker()
{
    std::string delimeter = "\r\n\r\n";

    size_t pos = this->m_request_buffer.find(delimeter);
    if (pos == std::string::npos)
        return 1;
    else
    {
        this->header_done = true;
        this->m_headers_buffer = this->m_request_buffer.substr(0, pos + delimeter.size());
        this->m_body_buffer = this->m_request_buffer.substr(pos + delimeter.size());
        this->m_request_buffer.clear();
    }
    return 0;
}

void Client::URI_parser(void)
{
    std::string &uri = this->m_request.m_URI;

    std::string temp_path;
    std::string temp_query_string;

    size_t pos;
    if ((pos = uri.find_first_of("?")) != std::string::npos)
    {
        temp_path = uri.substr(0, pos);
        temp_query_string = uri.substr(pos + 1);
    }
    else 
        temp_path = uri;

    std::string path;
    std::string query_string;

    for (size_t index = 0; index < temp_path.size(); ++index)
    {
        if (temp_path[index] == '%' && index + 2 < temp_path.size())
        {
            std::stringstream ss;
            ss << std::hex << temp_path.substr(index + 1, 2);
            int hex;
            ss >> hex;

            path.append(1, static_cast<char>(hex));
            index += 2;
        }
        else
            path.append(1, temp_path[index]);            
    }

    for (size_t index = 0; index < temp_query_string.size(); ++index)
    {
        if (temp_query_string[index] == '+')
            query_string.append(1, ' ');
        else
            query_string.append(1, temp_query_string[index]);            
    }

    this->m_request.m_URI = path;
    this->m_request.m_queryString = query_string;
}

void Client::headers_parser()
{
    Client &_client = ValuesSingleton::getValuesSingleton()._clients_map[this->m_client_fd];
    std::string delimeter = "\r\n";

    std::string check(_client.m_headers_buffer);
    size_t pos;

    pos = check.find(delimeter);
    if (pos != std::string::npos)
    {
        std::string request_line = check.substr(0, pos);
        check.erase(0, pos + delimeter.size());

        std::stringstream ss;
        ss << request_line;
        ss >> _client.m_request.m_method >> _client.m_request.m_URI >> _client.m_request.m_version;

        if (_client.m_request.m_method.empty() || _client.m_request.m_URI.empty() || _client.m_request.m_version.empty())
            _client.response_Error(400, true);

        if (_client.m_request.m_method != "GET" && _client.m_request.m_method != "POST" && _client.m_request.m_method != "DELETE")
        {
            if (_client.m_request.m_method == "HEAD")
                _client.response_Error(501, false);
            _client.response_Error(501, true);
        }

        if (_client.m_request.m_URI.find_first_not_of(www::Allowed_Chars) != std::string::npos)
            _client.response_Error(400, true);
        if (_client.m_request.m_URI[0] != '/')
            _client.response_Error(400, true);
        this->URI_parser();

        if (_client.m_request.m_version != "HTTP/1.0" && _client.m_request.m_version != "HTTP/1.1")
            _client.response_Error(505, true);
    }
    while ((pos = check.find(delimeter)) != std::string::npos)
    {
        std::string header_line = check.substr(0, pos);
        check.erase(0, pos + delimeter.size());
        
        size_t l_pos;
        if((l_pos = header_line.find(":")) != std::string::npos)
        {
            std::string key = header_line.substr(0, l_pos);
            header_line.erase(0, l_pos + 2);
            
            size_t ll_pos;
            while ((ll_pos = header_line.find(",")) != std::string::npos)
            {
                std::string value = header_line.substr(0, ll_pos);
                _client.m_request.m_headers[key].push_back(value);
                header_line.erase(0, ll_pos + 2);
            }
            if (!header_line.empty()) 
                _client.m_request.m_headers[key].push_back(header_line);
        }
    }
}

void Client::headers_implication()
{
    Client &_client = ValuesSingleton::getValuesSingleton()._clients_map[this->m_client_fd];
    
    if ("POST" == this->m_request.m_method)
    {
        this->need_body = true;

        const std::vector<std::string> *content_length = find_Value_inMap(_client.m_request.m_headers, "Content-Length");
        const std::vector<std::string> *transfer_encoding = find_Value_inMap(_client.m_request.m_headers, "Transfer-Encoding");
        
        bool l_isChunked = false;
        if (NULL != transfer_encoding)
        {
            if (std::find(transfer_encoding->begin(), transfer_encoding->end(), "chunked") != transfer_encoding->end())
                l_isChunked = true;
        }

        if ((NULL == content_length && false == l_isChunked) || (NULL != content_length && true == l_isChunked))
            this->response_Error(400, true);

        if (NULL != content_length)
        {
            if (std::strtol((*content_length)[0].c_str(), NULL, 10) < 0)
                this->response_Error(400, true);
            if (std::strtoul((*content_length)[0].c_str(), NULL, 10) > this->m_Myserver.m_client_max_body_size)
                this->response_Error(413, true);
            this->m_isChunked = false;
        }
        else
            this->m_isChunked = true;
    }
    else
        this->need_body = false;
}

static size_t hex_to_int(const std::string& string)
{
    std::stringstream ss;
    size_t value;

    ss << std::hex << string;
    ss >> value;

    return value;
}

bool Client::body_checker()
{
    if (this->m_isChunked == false)
    {
        if (!this->m_body_asFile->good())
            throw std::runtime_error("fstream.goood()");
        this->m_body_asFile->write(this->m_body_buffer.c_str(), this->m_body_buffer.size());
        this->m_body_asFile->flush();
        this->m_body_buffer.clear();

        const std::vector<std::string> *content_length = find_Value_inMap(this->m_request.m_headers, "Content-Length");
        size_t l_content_length = std::strtoul((*content_length)[0].c_str(), NULL, 10);
        
        std::streampos current_pos = this->m_body_asFile->tellg();
        this->m_body_asFile->seekg(0, std::ios::end);
        size_t size = this->m_body_asFile->tellg(); 

        if (size < l_content_length)
        {
            this->m_body_asFile->seekg(current_pos);
            return 1;
        }
        else
        {
            truncate(this->m_body_asFile_path.c_str(), l_content_length);
            return 0;
        }
    }
    else 
    {
        size_t pos = 0;
        std::string delimeter = "\r\n";
        std::string end = "0\r\n\r\n";
        
        pos = this->m_body_buffer.find(delimeter);
        if (pos == std::string::npos)
        return 1;
        else
        {
            while ((pos = this->m_body_buffer.find(delimeter)) != std::string::npos)
            {
                std::string chunk_size_s = this->m_body_buffer.substr(0, pos);
                size_t chunk_size = hex_to_int(chunk_size_s);
                
                if (chunk_size == 0)
                {
                    if (this->m_body_buffer == end)
                    {
                        if (this->m_body_buffer.size() > this->m_Myserver.m_client_max_body_size)
                            this->response_Error(413, true);
                        this->m_body_buffer.clear();
                        return 0;
                    }
                    return 1;
                }

                if (this->m_body_buffer.size() < (chunk_size_s.size() + chunk_size + (2 * delimeter.size())))
                    return 1;
                this->m_body_buffer.erase(0, pos + delimeter.size());

                if (!this->m_body_asFile->good())
                    throw std::runtime_error("fstream.good()");
                *this->m_body_asFile << this->m_body_buffer.substr(0, chunk_size);
                this->m_body_asFile->flush();
                
                this->m_body_buffer.erase(0, (chunk_size + delimeter.size()));
            }
            return 1;
        }
    }
    return 0;
}