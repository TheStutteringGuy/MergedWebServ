#include "Parser.hpp"
#include <vector>

static std::string root_check(std::string& root)
{
    std::string tmp(root);

    if (tmp[tmp.size() - 1] != '/')
        tmp += '/';

    return tmp;
}

void Parser(char **av)
{
    std::fstream file;

    file.open(av[1], std::ios::in | std::ios::ate);
    if (!file.is_open())
        throw std::invalid_argument("Cant open File");

    size_t size_file = file.tellg();
    if (size_file == 0)
        throw std::invalid_argument("File has No Content ?");
    file.seekg(0, std::ios::beg);

    std::vector<MyServerBlock> &servers_blocks = ValuesSingleton::getValuesSingleton().servers_blocks;

    int index = 0;
    bool inServer = false;
    std::string line;
    while (std::getline(file, line, '\n'))
    {
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

        if (line.empty())
            continue;
        if (line[0] == '#')
            continue;

        if (line == "server {")
        {
            inServer = true;
            servers_blocks.push_back(MyServerBlock());
            continue;
        }
        if (line == "}")
        {
            inServer = false;
            ++index;
            continue;
        }
        if (inServer == false) continue;

        size_t pos = line.find_first_of(";");
        std::string checking_line = line.substr(0, pos);

        std::stringstream ss(checking_line);

        std::string key;
        std::string value;
        std::vector<std::string> v_value;
        ss >> key;
        while (ss >> value)
            v_value.push_back(value);

        if (key == "listen")
        {
            if (v_value.empty()) 
                throw std::invalid_argument("Syntax Error");
            servers_blocks[index].m_directives["listen"].push_back(v_value[0]);
        }
        else if (key == "root")
        {
            if (v_value.empty())
            {
                servers_blocks[index].m_root = "./www";
                continue;
            }
            servers_blocks[index].m_root = root_check(v_value[0]);
        }
        else if (key == "error_page")
        {
            if (v_value.size() < 2)
                throw std::invalid_argument("Syntax Error");
            servers_blocks[index].m_error_pages[std::atoi(v_value[0].c_str())] = v_value[1];
        }
        else if (key == "client_max_body_size")
        {
            if (v_value.empty())
            {
                servers_blocks[index].m_client_max_body_size = 524288000;
                continue;
            }
            servers_blocks[index].m_client_max_body_size = std::strtoul(v_value[0].c_str(), NULL, 10);
        }
        else if (key == "location" && v_value[1] == "{")
        {
            MyLocationBlock &l_location = servers_blocks[index].m_locationBlocks[v_value[0]];
            while (std::getline(file, line, '\n'))
            {
                line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
                line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

                if (line.empty())
                    continue;
                if (line[0] == '#')
                    continue;
                if (line == "}")
                    break;

                size_t pos = line.find_first_of(";");
                std::string checking_line = line.substr(0, pos);

                std::stringstream ss(checking_line);

                std::string key;
                std::string value;
                std::vector<std::string> v_value;

                 ss >> key;
                while (ss >> value)
                    v_value.push_back(value);
                if (key == "root")
                {
                    if (v_value.empty())
                    {
                        l_location.root = std::string();
                        continue;
                    }
                    l_location.root = root_check(v_value[0]);
                }
                else if (key == "allow_methods")
                {
                    if (v_value.empty())
                    {
                        l_location.allowed_methods.push_back("GET");
                        l_location.allowed_methods.push_back("POST");
                        l_location.allowed_methods.push_back("DELETE");
                        continue;
                    }
                    for (size_t index = 0; index < v_value.size(); ++index)
                        l_location.allowed_methods.push_back(v_value[index]);
                }
                else if (key == "index")
                {
                    if (v_value.empty())
                    {
                        l_location.index = std::string();
                        continue;
                    }
                    l_location.index = v_value[0];
                }
                else if (key == "autoindex")
                {
                    if (v_value.empty())
                    {
                        l_location.autoindex = false;
                        continue;
                    }
                    if (v_value[0] == "on")
                        l_location.autoindex = true;
                    else
                        l_location.autoindex = false;
                }
                else if (key == "return")
                {
                    if (v_value.size() >= 2)
                    {
                        l_location.is_Red = true;
                        size_t status_code = strtol(v_value[0].c_str(), NULL, 10);
                        if (status_code != 301 && status_code != 302)
                            throw std::invalid_argument("Syntax Error");
                        l_location.redirection[status_code] = v_value[1];
                    }
                }
                continue;
            }
        }
        continue;
    }
}