/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aahlaqqa <aahlaqqa@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/04 20:41:13 by ahmed             #+#    #+#             */
/*   Updated: 2025/11/03 17:31:36 by aahlaqqa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser.hpp"

Parser::Parser() {

};

Parser::~Parser() {

};

LocationBlock::LocationBlock() : autoindex(false), is_redirection(false), is_cgi(false), upload(false) {};

LocationBlock::~LocationBlock() {};

ServerBlock::ServerBlock() : clientSizeBody(1048576) {};

ServerBlock::~ServerBlock() {};

const char *Parser::IncorrArgc::what() const throw()
{
    return "Incorrect number of arguments !!";
}

const char *Parser::ErrorFile::what() const throw()
{
    return "while openning the file !!";
};

const char *Parser::ServerError::what() const throw()
{
    return "No server in top of the config file !!";
};

const char *Parser::ParsError::what() const throw()
{
    return "Expected an open brace after the block !!";
};

const char *Parser::ParsErrorEnd::what() const throw()
{
    return "Expected closed barce at the end of the block !!";
};

const char *Parser::ParsLocation::what() const throw()
{
    return "Expected a Path after the location block !!";
};

const char *Parser::ParsSemiColn::what() const throw()
{
    return "Unexpected semicoln !!";
};

const char *Parser::Unknown::what() const throw()
{
    return "Unknown directive !!";
};

const char *Parser::NoListen::what() const throw()
{
    return "Listen directive is missing !!";
};

const char *Parser::NoRoot::what() const throw()
{
    return "Root directive is missing !!";
};

const char *Parser::InvalidMthode::what() const throw()
{
    return "Invalid HTTP method !!";
};

const char *Parser::InvalidCgi::what() const throw()
{
    return "Invalid CGI method !!";
};

std::vector<ServerBlock> &Parser::getServerBlock()
{
    return (server_blocks);
};

const std::vector<ServerBlock> &Parser::getServerBlock() const
{
    return (server_blocks);
};

void Parser::trim_spaces(std::string &line)
{
    line.erase(0, line.find_first_not_of("\n \t"));
    line.erase(line.find_last_not_of("\n \t") + 1);
};

void Parser::parsToken(const std::string &line)
{
    std::string token;
    char c;

    for (size_t i = 0; i < line.length(); i++)
    {
        c = line[i];
        if (c == '{' || c == '}' || c == ';' || std::isspace(c))
        {
            if (!token.empty())
            {
                tokens.push_back(token);
                token.clear();
            }
            if (c == '{' || c == '}' || c == ';')
            {
                std::string special_char(1, c);
                tokens.push_back(special_char);
            }
        }
        else
        {
            token += c;
        }
    }
    if (!token.empty())
        tokens.push_back(token);
};

const std::map<std::string, Parser::Lexical> &Parser::get_token()
{
    static std::map<std::string, Parser::Lexical> tokens;

    if (tokens.empty())
    {
        tokens["server"] = Parser::SERVER;
        tokens["listen"] = Parser::LISTEN;
        tokens["server_name"] = Parser::SERVER_NAME;
        tokens["host"] = Parser::HOST;
        tokens["root"] = Parser::ROOT;
        tokens["index"] = Parser::INDEX;
        tokens["error_page"] = Parser::ERROR;
        tokens["location"] = Parser::LOCATION;
        tokens["allow_methods"] = Parser::METHODS;
        tokens["autoindex"] = Parser::AUTOINDEX;
        tokens["return"] = Parser::RETURN;
        tokens["cgi_path"] = Parser::CGI;
        tokens["client_max_body_size"] = Parser::CLIENT_MAX_BODY_SIZE;    }
    return (tokens);
};

Parser::Lexical Parser::get_token_type(const std::string &token)
{
    if (token == "{")
        return (Parser::OPEN_BRACET);
    if (token == "}")
        return (Parser::CLOSED_BRACET);
    if (token == ";")
        return (Parser::SEMICOLN);

    const std::map<std::string, Parser::Lexical> &tokenMap = get_token();
    std::map<std::string, Parser::Lexical>::const_iterator it = tokenMap.find(token);
    if (it != tokenMap.end())
        return (it->second);
    return (Parser::ARGUMENT);
};

void Parser::Tokenization(int argc, char **argv)
{
    if (argc != 2)
        throw IncorrArgc();

    std::ifstream file(argv[1]);
    if (!file.is_open())
        throw ErrorFile();

    std::string line;
    while (getline(file, line))
    {
        trim_spaces(line);
        if (line[0] == '#')
            continue;
        parsToken(line);
    }
    for (size_t i = 0; i < tokens.size(); i++)
    {
        Lexical type = get_token_type(tokens[i]);
        lexical_vector.push_back(std::make_pair(tokens[i], type));
    }
    return;
};

void Parser::parsConfig()
{
    size_t i;

    i = 0;
    while (i < lexical_vector.size())
    {
        if (lexical_vector[i].second == SERVER)
        {
            i++;
            server_blocks.push_back(parsServerBlock(i));
        }
        else
            throw ServerError();
    }
};

ServerBlock Parser::parsServerBlock(size_t &i)
{
    ServerBlock server;

    if (lexical_vector[i].second != OPEN_BRACET)
        throw ParsError();
    i++;
    while (i < lexical_vector.size() && lexical_vector[i].second != CLOSED_BRACET)
    {
        if (lexical_vector[i].second == LOCATION)
            server.locationBlocks.push_back(parsLocationBlock(i));
        else
            server.directives.push_back(parsDirectiveBlock(i));
    }
    if (i >= lexical_vector.size() || lexical_vector[i].second != CLOSED_BRACET)
        throw ParsErrorEnd();
    i++;
    return (server);
};

LocationBlock Parser::parsLocationBlock(size_t &i)
{
    LocationBlock location;

    i++;
    if (lexical_vector[i].second != ARGUMENT)
        throw ParsLocation();
    location.path = lexical_vector[i].first;
    i++;

    if (lexical_vector[i].second != OPEN_BRACET)
        throw ParsError();
    i++;
    while (i < lexical_vector.size() && lexical_vector[i].second != CLOSED_BRACET)
        location.directives.push_back(parsDirectiveBlock(i));
    if (i >= lexical_vector.size() || lexical_vector[i].second != CLOSED_BRACET)
        throw ParsErrorEnd();
    i++;
    return (location);
};

Directive Parser::parsDirectiveBlock(size_t &i)
{
    if (lexical_vector[i].second == SEMICOLN)
        throw ParsSemiColn();

    Directive directive;
    directive.name = lexical_vector[i].first;
    i++;

    while (i < lexical_vector.size() && lexical_vector[i].second == ARGUMENT)
    {
        directive.values.push_back(lexical_vector[i].first);
        i++;
    }

    if (i >= lexical_vector.size() || lexical_vector[i].second != SEMICOLN)
        throw ParsSemiColn();
    i++;
    return (directive);
};
void Parser::validatConfig() const
{
    std::set<std::string> validDirectives;
    validDirectives.insert("listen");
    validDirectives.insert("server_name");
    validDirectives.insert("host");
    validDirectives.insert("root");
    validDirectives.insert("index");
    validDirectives.insert("error_page");
    validDirectives.insert("allow_methods");
    validDirectives.insert("autoindex");
    validDirectives.insert("return");
    validDirectives.insert("cgi");
    validDirectives.insert("client_max_body_size");
    validDirectives.insert("upload");
    validDirectives.insert("upload_path");
    
    for (size_t i = 0; i < server_blocks.size(); i++)
    {
        const ServerBlock &server = server_blocks[i];
        bool has_listen = false;
        bool has_root = false;

        for (size_t j = 0; j < server.directives.size(); j++)
        {
            const std::string &name = server.directives[j].name;
            if (validDirectives.find(name) == validDirectives.end())
                throw Unknown();

            if (name == "listen")
                has_listen = true;
            else if (name == "root")
                has_root = true;
        }
        if (!has_listen)
            throw NoListen();
        else if (!has_root)
            throw NoRoot();

        for (size_t y = 0; y < server.locationBlocks.size(); y++)
        {
            const LocationBlock &location = server.locationBlocks[y];
            for (size_t x = 0; x < location.directives.size(); x++)
            {
                const Directive &dir = location.directives[x];

                if (validDirectives.find(dir.name) == validDirectives.end())
                    throw Unknown();
                if (dir.name == "allow_methods")
                {
                    for (size_t k = 0; k < dir.values.size(); k++)
                    {
                        const std::string &method = dir.values[k];
                        if (method != "GET" && method != "POST" && method != "DELETE")
                        {
                            throw InvalidMthode();
                        }
                    }
                }
            }
        }
    }
};