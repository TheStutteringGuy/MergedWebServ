#pragma once
#include "../Parsing/configProcessor.hpp"
#include "../Parsing/parser.hpp"

namespace API {
    void ServerBlock_Parser(std::vector<ServerBlock> &servers_blocks);
    int Webserver(void);
}