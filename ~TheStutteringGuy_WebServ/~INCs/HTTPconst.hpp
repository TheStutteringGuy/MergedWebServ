#pragma once
#include "../~INCs/'Standards'.hpp"

struct FileContent_type
{
private:
    std::map<std::string, std::string>  m_FileContent_type;

public:
    FileContent_type() 
    {
        m_FileContent_type["html"] = "text/html";
        m_FileContent_type["txt"] = "text/plain";
        m_FileContent_type["css"] = "text/css";
        m_FileContent_type["xml"] = "text/xml";
        m_FileContent_type["csv"] = "text/csv";
        
        // Application formats
        m_FileContent_type["json"] = "application/json";
        m_FileContent_type["pdf"] = "application/pdf";
        m_FileContent_type["zip"] = "application/zip";
        m_FileContent_type["bin"] = "application/octet-stream";
        m_FileContent_type["doc"] = "application/msword";
        m_FileContent_type["docx"] = "application/vndopenxmlformats-officedocumentwordprocessingmldocument";
        
        // Image formats
        m_FileContent_type["jpg"] = "image/jpeg";
        m_FileContent_type["jpeg"] = "image/jpeg";
        m_FileContent_type["png"] = "image/png";
        m_FileContent_type["gif"] = "image/gif";
        m_FileContent_type["webp"] = "image/webp";
        m_FileContent_type["ico"] = "image/x-icon";
        
        // Audio formats
        m_FileContent_type["mp3"] = "audio/mpeg";
        m_FileContent_type["wav"] = "audio/wav";
        
        // Video formats
        m_FileContent_type["mp4"] = "video/mp4";
        m_FileContent_type["mpeg"] = "video/mpeg";
    }

    std::string getContent_type(const std::string& key) const 
    {
        std::map<std::string, std::string>::const_iterator it = m_FileContent_type.find(key);
        return (it != m_FileContent_type.end()) ? it->second : "application/octet-stream";
    }
};

struct Reverse_FileContent_type
{
private:
    std::map<std::string, std::string>  m_Reverse_FileContent_type;

public:
    Reverse_FileContent_type() 
    {
        // Text Formats
        m_Reverse_FileContent_type["text/html"] = ".html";
        m_Reverse_FileContent_type["text/plain"] = ".txt";
        m_Reverse_FileContent_type["text/css"] = ".css";
        m_Reverse_FileContent_type["text/javascript"] = ".js";
        m_Reverse_FileContent_type["text/xml"] = ".xml";
        m_Reverse_FileContent_type["text/csv"] = ".csv";
        
        // Application formats
        m_Reverse_FileContent_type["application/json"] = ".json";
        m_Reverse_FileContent_type["application/xml"] = ".xml";
        m_Reverse_FileContent_type["application/pdf"] = ".pdf";
        m_Reverse_FileContent_type["application/zip"] = ".zip";
        m_Reverse_FileContent_type["application/octet-stream"] = ".bin";
        m_Reverse_FileContent_type["application/javascript"] = ".js";
        m_Reverse_FileContent_type["application/msword"] = ".doc";
        m_Reverse_FileContent_type["application/vnd.openxmlformats-officedocument.wordprocessingml.document"] = ".docx";
        
        // Image formats
        m_Reverse_FileContent_type["image/jpeg"] = ".jpg";
        m_Reverse_FileContent_type["image/png"] = ".png";
        m_Reverse_FileContent_type["image/gif"] = ".gif";
        m_Reverse_FileContent_type["image/webp"] = ".webp";
        m_Reverse_FileContent_type["image/x-icon"] = ".ico";
        m_Reverse_FileContent_type["image/vnd.microsoft.icon"] = ".ico";
        
        // Audio formats
        m_Reverse_FileContent_type["audio/mpeg"] = ".mp3";
        m_Reverse_FileContent_type["audio/wav"] = ".wav";
        
        // Video formats
        m_Reverse_FileContent_type["video/mp4"] = ".mp4";
        m_Reverse_FileContent_type["video/mpeg"] = ".mpeg";
        m_Reverse_FileContent_type["video/quicktime"] = ".mov";
        m_Reverse_FileContent_type["video/x-msvideo"] = ".avi";
        
    }

    std::string getContent_type(const std::string& key) const 
    {
        std::map<std::string, std::string>::const_iterator it = m_Reverse_FileContent_type.find(key);
        return (it != m_Reverse_FileContent_type.end()) ? it->second : ".bin";
    }
};

struct HTTPstatus_phrase
{
private:
    std::map<std::string, std::string>  m_HTTPstatus_phrase;

public:
    HTTPstatus_phrase() 
    {
        m_HTTPstatus_phrase["200"] = "OK";
        m_HTTPstatus_phrase["201"] = "Created";
        m_HTTPstatus_phrase["202"] = "Accepted";
        m_HTTPstatus_phrase["204"] = "No Content";
        m_HTTPstatus_phrase["301"] = "Moved Permanently";
        m_HTTPstatus_phrase["302"] = "Moved Temporarily";
        m_HTTPstatus_phrase["304"] = "Not Modified";
        m_HTTPstatus_phrase["400"] = "Bad Request";
        m_HTTPstatus_phrase["401"] = "Unauthorized";
        m_HTTPstatus_phrase["403"] = "Forbidden";
        m_HTTPstatus_phrase["404"] = "Not Found";
        m_HTTPstatus_phrase["405"] = "Method Not Allowed";
        m_HTTPstatus_phrase["408"] = "Request Timeout";
        m_HTTPstatus_phrase["411"] = "Length Required";
        m_HTTPstatus_phrase["413"] = "Request Entity Too Large";
        m_HTTPstatus_phrase["414"] = "URI Too Long";
        m_HTTPstatus_phrase["415"] = "Unsupported Media Type";
        m_HTTPstatus_phrase["429"] = "Too Many Requests";
        m_HTTPstatus_phrase["500"] = "Internal Server Error";
        m_HTTPstatus_phrase["501"] = "Not Implemented";
        m_HTTPstatus_phrase["502"] = "Bad Gateway";
        m_HTTPstatus_phrase["503"] = "Service Unavailable";
        m_HTTPstatus_phrase["504"] = "Gateway Timeout";
        m_HTTPstatus_phrase["505"] = "HTTP Version Not Supported";
    }

    std::string getStatusPhrase(const std::string& key) const 
    {
        std::map<std::string, std::string>::const_iterator it = m_HTTPstatus_phrase.find(key);
        return (it != m_HTTPstatus_phrase.end()) ? it->second : "Unknown Status";
    }
};

// session struct
struct session
{
    std::string user_id;
    time_t experation_time;
};