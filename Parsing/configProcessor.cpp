/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configProcessor.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aahlaqqa <aahlaqqa@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 16:54:24 by ahmed             #+#    #+#             */
/*   Updated: 2025/11/10 11:21:56 by aahlaqqa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "configProcessor.hpp"
#include "parser.hpp"

ConfigProcessor::ConfigProcessor() {

};

ConfigProcessor::~ConfigProcessor() {

};

const char *ConfigProcessor::InvalidPortException::what() const throw()
{
    return "Invalid Port number !!";
};

const char *ConfigProcessor::DuplicatePortEXception::what() const throw()
{
    return "Duplicate Port detected !!";
};

const char *ConfigProcessor::InvalidPathException::what() const throw()
{
    return "Invalid Path detected !!";
};

const char *ConfigProcessor::InvalidDirectiveException::what() const throw()
{
    return "Invalid directive is detected !!";
};

const char *ConfigProcessor::InvalidClientBodySize::what() const throw()
{
    return "Client max body warning failed !";
};

const char *ConfigProcessor::InvalidReturnCode::what() const throw()
{
    return "Invalid return code !";
};

const char *ConfigProcessor::InvalidUploadPath::what() const throw()
{
    return "Invalid upload path !";
};

void ConfigProcessor::processServerBlock(std::vector<ServerBlock> &blocks)
{
    for (size_t i = 0; i < blocks.size(); i++)
    {
        setDefaultValue(blocks[i]);
        processServerDirective(blocks[i]);
        for (size_t j = 0; j < blocks[i].locationBlocks.size(); j++)
        {
            setLocation(blocks[i].locationBlocks[j], blocks[i]);
            processLocationDirective(blocks[i].locationBlocks[j], blocks[i]);
        }
    }
    validateConfig(blocks);
};

size_t ConfigProcessor::parseClientBodySize(const std::string &value)
{
    if (value.empty())
        throw InvalidClientBodySize();
    size_t i;
    i = 0;
    while (i < value.size() && (std::isdigit(value[i]) || value[i] == '.'))
        i++;
    std::string numPart;
    std::string unitPart;
    numPart = value.substr(0, i);
    unitPart = value.substr(i);
    for (size_t j = 0; j < unitPart.size(); j++)
        unitPart[j] = std::tolower(unitPart[j]);
    std::istringstream ss(numPart);
    double num = 0;
    ss >> num;
    if (ss.fail())
        throw InvalidClientBodySize();
    size_t bytes = 0;
    if (unitPart.empty() || unitPart == "b")
        bytes = static_cast<size_t>(num);
    else if (unitPart == "kb" || unitPart == "k")
        bytes = static_cast<size_t>(num * 1024);
    else if (unitPart == "mb" || unitPart == "m")
        bytes = static_cast<size_t>(num * 1024 * 1024);
    else if (unitPart == "gb" || unitPart == "g")
        bytes = static_cast<size_t>(num * 1024 * 1024 * 1024);
    else
        throw std::invalid_argument("Invalid size unit: " + unitPart);

    return (bytes);
};

void ConfigProcessor::processServerDirective(ServerBlock &blocks)
{
    for (size_t i = 0; i < blocks.directives.size(); i++)
    {
        const Directive &dir = blocks.directives[i];
        if (dir.name == "host")
        {
            if (!dir.values.empty())
                blocks.host = dir.values[0];
        }
        else if (dir.name == "server_name")
        {
            if (!dir.values.empty())
                blocks.server_name = dir.values[0];
        }
        else if (dir.name == "root")
        {
            if (!dir.values.empty())
                blocks.root = dir.values[0];
        }
        else if (dir.name == "index")
        {
            if (!dir.values.empty())
                blocks.index = dir.values[0];
        }
        else if (dir.name == "error_page")
        {
            if (dir.values.empty())
                std::cout << "Eroor : error_pages required at least one value" << std::endl;
            else if (dir.values.size() == 1 && is_Num(dir.values[0]))
                std::cout << "Error : error pages missing a path after code : " << dir.values[0] << std::endl;
            else if (dir.values.size() == 2)
            {
                std::string last_value;
                last_value = dir.values[1];
                if (is_Num(dir.values[0]))
                {
                    int code;
                    code = std::atoi(dir.values[0].c_str());
                    blocks.error_pages[code] = last_value;
                }
                else
                    std::cout << "WARNING : Invalid code " << dir.values[0] << std::endl;
            }
        }
        else if (dir.name == "client_max_body_size")
        {
            if (!dir.values.empty())
                blocks.clientSizeBody = parseClientBodySize(dir.values[0]);
        }
        else if (dir.name == "allow_methods")
        {
            if (!dir.values.empty())
            {
                for (size_t i = 0; i < dir.values.size(); i++)
                {
                    if (dir.values[i] != "GET" && dir.values[i] != "POST" && dir.values[i] != "DELETE")
                    {
                        throw Parser::InvalidMthode();
                    }
                }
            }
        }
    }
};

bool ConfigProcessor::is_Num(const std::string &str)
{
    if (str.empty())
        return (false);
    for (std::string::size_type i = 0; i < str.size(); i++)
    {
        if (!std::isdigit(str[i]))
            return (false);
    }
    return (true);
};

void ConfigProcessor::processLocationDirective(LocationBlock &location, const ServerBlock &server)
{
    (void)server;
    location.is_cgi = false;
    location.is_redirection = false;
    location.upload = false;
    location.upload_path = "";
    for (size_t i = 0; i < location.directives.size(); i++)
    {
        const Directive &dir = location.directives[i];
        if (dir.name == "root")
        {
            if (!dir.values.empty())
                location.root = dir.values[0];
        }
        else if (dir.name == "index")
        {
            if (!dir.values.empty())
                location.index = dir.values[0];
        }
        else if (dir.name == "allow_methods")
            location.allowed_methods = dir.values;
        else if (dir.name == "autoindex")
        {
            if (!dir.values.empty())
                location.autoindex = (dir.values[0] == "on");
        }
        else if (dir.name == "upload")
        {
            if (!dir.values.empty() && dir.values[0] == "on")
                location.upload = true;
        }
        else if (dir.name == "upload_path")
        {
            if (!dir.values.empty())
                location.upload_path = dir.values[0];
        }
        else if (dir.name == "return")
        {
            if (dir.values.size() >= 2 && ((dir.values[0] == "302") || (dir.values[0] == "301")))
            {
                location.is_redirection = true;
                int code;
                std::string url;
                code = std::atoi(dir.values[0].c_str());
                url = dir.values[1];
                location.redirect_url[code] = url;
            }
            else
            {
                throw ConfigProcessor::InvalidReturnCode();
            }
        }
        else if (dir.name == "cgi")
        {
            if (dir.values.size() >= 2)
            {
                location.is_cgi = true;
                std::string extention;
                std::string path;
                extention = dir.values[0];
                path = dir.values[1];
                location.cgi[extention] = path;
            }
            else
            {
                throw Parser::InvalidCgi();
            }
        }
    }

    if (location.upload == true)
    {
        if (location.upload_path.empty())
            throw InvalidUploadPath();
        std::string full_path = handlePathUpload(server.root, location.upload_path);
        if (!existDirectory(full_path))
            throw InvalidUploadPath();
    }
};

void ConfigProcessor::validateConfig(const std::vector<ServerBlock> &blocks)
{
    validatePath(blocks);
};

void ConfigProcessor::validatePath(const std::vector<ServerBlock> &blocks)
{
    for (size_t i = 0; i < blocks.size(); i++)
    {
        const ServerBlock &server = blocks[i];
        if (!existDirectory(server.root))
        {
            std::cout << "ERROR: Server root directory does not exist: " << server.root << std::endl;
            throw InvalidPathException();
        }
        for (size_t j = 0; j < server.locationBlocks.size(); j++)
        {
            const LocationBlock &location = server.locationBlocks[j];
            if (!existDirectory(location.root))
            {
                std::cout << "ERROR: Location root directory does not exist: " << location.root << std::endl;
                throw InvalidPathException();
            }
        }
    }
};

void ConfigProcessor::setDefaultValue(ServerBlock &server)
{
    if (server.server_name.empty())
        server.server_name = "localhost";
};

void ConfigProcessor::setLocation(LocationBlock &location, const ServerBlock &server)
{
    if (location.root.empty())
        location.root = server.root;
    if (location.index.empty())
        location.index = server.index;
    if (location.allowed_methods.empty())
    {
        location.allowed_methods.push_back("GET");
        location.allowed_methods.push_back("POST");
        location.allowed_methods.push_back("DELETE");
    }
};

int ConfigProcessor::stringToInt(const std::string &str)
{
    std::stringstream ss(str);
    int res;

    ss >> res;
    if (ss.fail())
        throw InvalidDirectiveException();
    return (res);
};

bool ConfigProcessor::existFile(const std::string &path)
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0)
        return (false);
    if (S_ISREG(buffer.st_mode))
        return (true);
    else
        return (false);
};

bool ConfigProcessor::existDirectory(const std::string &path)
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0)
        return (false);
    if (S_ISDIR(buffer.st_mode))
        return (true);
    else
        return (false);
};

std::string ConfigProcessor::handlePath(const std::string &base, const std::string &path)
{
    if (path.empty())
        return (base);
    if (path[0] == '/')
        return (path);
    std::string res = base;
    if (!res.empty() && res[res.length() - 1] != '/')
        res += "/";
    res += path;
    return (res);
};

std::string ConfigProcessor::handlePathUpload(const std::string &base, const std::string &path)
{
    if (path.empty())
        return (base);
    std::string res = base;
    if (!res.empty() && res[res.length() - 1] != '/')
        res += '/';   
    std::string final_path = path;
    if (!final_path.empty() && final_path[0] == '/')
        final_path.erase(0, 1);
    
    res += final_path;
    return (res);
};