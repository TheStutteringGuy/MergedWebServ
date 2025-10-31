#pragma once
#include "../~INCs/'Standards'.hpp"

namespace www {
    #define True 21
    typedef int fd_t;
    
    #define MAX_EVENTS 32
    #define MAX_CONNECTIONS 1024

    #define ReadingSize 70656
    #define WritingSize 1048576
    
    #define CONTINUE 42
    #define END 21
    
    extern const std::string Allowed_Chars;

    #define cache ".Cache/"
    #define upload_dir "Upload/"
    
    extern const std::string RED;
    extern const std::string GREEN;
    extern const std::string YELLOW;
    extern const std::string BLUE;
    extern const std::string MAGENTA;
    extern const std::string CYAN;
    extern const std::string RESET;

    extern bool SHUTDOWN;
}