// /* ************************************************************************** */
// /*                                                                            */
// /*                                                        :::      ::::::::   */
// /*   webServer.hpp                                      :+:      :+:    :+:   */
// /*                                                    +:+ +:+         +:+     */
// /*   By: aahlaqqa <aahlaqqa@student.42.fr>          +#+  +:+       +#+        */
// /*                                                +#+#+#+#+#+   +#+           */
// /*   Created: 2025/07/31 10:23:21 by aahlaqqa          #+#    #+#             */
// /*   Updated: 2025/07/31 20:30:58 by aahlaqqa         ###   ########.fr       */
// /*                                                                            */
// /* ************************************************************************** */

// #ifndef WEBSERVER_HPP
// #define WEBSERVER_HPP

// #include <vector>
// #include <map>
// #include <iostream>
// #include <unistd.h>
// #include <string>
// #include <cstring>
// #include <cstdio>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <fcntl.h>
// #include <cerrno>
// #include <arpa/inet.h>
// #include <poll.h>
// #include "../Parsing/parser.hpp"
// #include "client.hpp"
// #include "webServMacro.hpp"

// struct ListeningSocket
// {
//     int fd;
//     std::string host;
//     int port;
// };

// class WebServer
// {
// public:
//     WebServer();
//     WebServer(const std::vector<ServerBlock> &servers);
//     ~WebServer();

//     void run();

// private:
//     std::vector<ListeningSocket> listeningSocket;
//     std::vector<struct pollfd> pollfds;
//     std::map<int, Client> clients; // like <client_fd, Client>
//     std::vector<ServerBlock> server_block;

//     bool setupListeningSocket(const std::vector<ServerBlock> &servers);
//     bool setNonBlocking(int fd);
//     void closeSockets();
//     void mainEventLoop();

//     void handleNewConnections(size_t listen_idx);
//     void handleClientRead(int fd);
//     void handleClientWrite(int fd);
//     void handleDeleteMethod(int fd);
//     void handleGetMethod(int fd);
//     //add
//     void handlePostMethod(int fd);
//     void handleCGIRequest(int fd);
//     bool isCGIRequest(const std::string& path, const LocationBlock* location);
//     void executeCgiScript(int fd, const ServerBlock* server, const LocationBlock* location);
//     void handleFileUpload(int fd, const LocationBlock* location);
//     // std::string getCGIInterpreter(const std::string& path, const LocationBlock *location);

//     //
//     std::string getContentType(const std::string &path);
//     const ServerBlock *findServerBlock(int fd);
//     const LocationBlock* findLocationBlock(const ServerBlock& server, const std::string& path);
//     std::string resolveFilePath(const std::string& root, const std::string& url_path);
//     std::string buildHttpResponse(const std::string& filepath, const ServerBlock& server);
//     std::string buildHttpResponse(int code, const std::string& status, const std::string& body); // Overloading function
// };

// #endif