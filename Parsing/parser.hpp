/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahmed <ahmed@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:11:52 by aahlaqqa          #+#    #+#             */
/*   Updated: 2025/10/08 15:48:36 by ahmed            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <exception>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <utility>
#include <cctype>
#include <map>
#include <set>
#include "configProcessor.hpp"

struct Directive
{
    std::string name;  // ex: listen
    std::vector<std::string> values; // ex: 8080
};

struct LocationBlock
{
    LocationBlock();
    ~LocationBlock();

    std::string path;
    std::vector<Directive> directives;

    std::string root;
    std::string index;
    std::vector<std::string> allowed_methods;
    bool autoindex;
    std::map<int,std::string> redirect_url;
    std::string cgi_path;
    std::vector<std::string> cgi_extention;
    bool is_redirection;
    bool is_cgi;
};

struct ServerBlock
{
    ServerBlock();
    ~ServerBlock();

    std::vector<Directive> directives;
    std::vector<LocationBlock> locationBlocks;

    int port;
    std::string server_name;
    std::string host;
    std::string root;
    std::string index;
    std::map<int, std::string> error_pages;
    size_t clientSizeBody;
};

class Parser
{
public:
    Parser();
    ~Parser();

    // Parsing functions

    void Tokenization(int argc, char **argv);
    void trim_spaces(std::string &line);
    void parsToken(const std::string &line);
    void parsConfig();
    ServerBlock parsServerBlock(size_t &i);
    LocationBlock parsLocationBlock(size_t &i);
    Directive parsDirectiveBlock(size_t &i);

    std::vector<ServerBlock> &getServerBlock();
    const std::vector<ServerBlock> &getServerBlock() const;

    void printConfig();

    class IncorrArgc : public std::exception
    {
        virtual const char *what() const throw();
    };

    class ErrorFile : public std::exception
    {
        virtual const char *what() const throw();
    };

    class ServerError : public std::exception
    {
        virtual const char *what() const throw();
    };

    class ParsError : public std::exception
    {
        virtual const char *what() const throw();
    };

    class ParsErrorEnd : public std::exception
    {
        virtual const char *what() const throw();
    };

    class ParsLocation : public std::exception
    {
        virtual const char *what() const throw();
    };

    class ParsSemiColn : public std::exception
    {
        virtual const char *what() const throw();
    };

    class Unknown : public std::exception
    {
        virtual const char *what() const throw();
    };

    class NoListen : public std::exception
    {
        virtual const char *what() const throw();
    };

    class NoRoot : public std::exception
    {
        virtual const char *what() const throw();
    };

    class InvalidMthode : public std::exception
    {
        virtual const char *what() const throw();
    };

    enum Lexical
    {
        LISTEN,
        SERVER_NAME,
        HOST,
        ROOT,
        INDEX,
        ERROR,
        CGI_PATH,
        CGI_EX,
        LOCATION,
        METHODS,
        AUTOINDEX,
        SERVER,
        RETURN,
        CLOSED_BRACET,
        OPEN_BRACET,
        SEMICOLN,
        ARGUMENT,
        CLIENT_MAX_BODY_SIZE,
        UNKNOWN
    };

    // Lexical functions
    Parser::Lexical get_token_type(const std::string &token);
    const std::map<std::string, Parser::Lexical> &get_token();
    void validatConfig() const;

private:
    std::vector<std::string> tokens;
    std::vector<std::pair<std::string, Lexical> > lexical_vector;

    std::vector<ServerBlock> server_blocks;
};

#endif