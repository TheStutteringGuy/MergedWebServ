/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configProcessor.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aahlaqqa <aahlaqqa@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 16:54:22 by ahmed             #+#    #+#             */
/*   Updated: 2025/10/08 21:55:07 by aahlaqqa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPROCESSOR_HPP
#define CONFIGPROCESSOR_HPP

#include <iostream>
#include <sstream>
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
#include <sys/stat.h>

struct ServerBlock;
struct LocationBlock;

class ConfigProcessor
{
public:
  ConfigProcessor();
  ~ConfigProcessor();

  void processServerBlock(std::vector<ServerBlock> &blocks);
  void validateConfig(const std::vector<ServerBlock> &blocks);

private:
  void processServerDirective(ServerBlock &blocks);
  void processLocationDirective(LocationBlock &location, const ServerBlock &server);
  void validatePort(const std::vector<ServerBlock> &blocks);
  void validatePath(const std::vector<ServerBlock> &blocks);
  void setDefaultValue(ServerBlock &server);
  void setLocation(LocationBlock &location, const ServerBlock &server);
  bool is_Num(const std::string &str);
  size_t parseClientBodySize(const std::string &value);

  int stringToInt(const std::string &str);
  // bool isValidPort(int port);
  bool existFile(const std::string &path);
  bool existDirectory(const std::string &path);
  std::string handlePath(const std::string &base, const std::string &path);

  class InvalidPortException : public std::exception
  {
    virtual const char *what() const throw();
  };

  class DuplicatePortEXception : public std::exception
  {
    virtual const char *what() const throw();
  };

  class InvalidPathException : public std::exception
  {
    virtual const char *what() const throw();
  };

  class InvalidDirectiveException : public std::exception
  {
    virtual const char *what() const throw();
  };

  class InvalidClientBodySize : public std::exception
  {
    virtual const char *what() const throw();
  };
};

#endif