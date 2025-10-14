// /* ************************************************************************** */
// /*                                                                            */
// /*                                                        :::      ::::::::   */
// /*   webServer.cpp                                      :+:      :+:    :+:   */
// /*                                                    +:+ +:+         +:+     */
// /*   By: aahlaqqa <aahlaqqa@student.42.fr>          +#+  +:+       +#+        */
// /*                                                +#+#+#+#+#+   +#+           */
// /*   Created: 2025/07/26 22:11:38 by aahlaqqa          #+#    #+#             */
// /*   Updated: 2025/08/01 23:39:17 by aahlaqqa         ###   ########.fr       */
// /*                                                                            */
// /* ************************************************************************** */

// #include "webServer.hpp"

// WebServer::WebServer() {};

// WebServer::WebServer(const std::vector<ServerBlock> &servers)
// 	: server_block(servers) // Store the config!
// {
// 	if (!setupListeningSocket(servers))
// 	{
// 		std::cerr << "Failed to set up listening socket !!" << std::endl;
// 		throw Parser::Unknown();
// 	}
// }

// WebServer::~WebServer()
// {
// 	closeSockets();
// };

// bool WebServer::setNonBlocking(int fd)
// {
// 	int flag;
// 	flag = fcntl(fd, F_GETFL, 0);
// 	if (flag < 0)
// 		return (false);
// 	int res;
// 	res = fcntl(fd, F_SETFL, flag | O_NONBLOCK);
// 	if (res < 0)
// 		return (false);
// 	return (true);
// };

// bool WebServer::setupListeningSocket(const std::vector<ServerBlock> &servers)
// {
// 	// first here i loop over all my server config
// 	for (size_t i = 0; i < servers.size(); i++)
// 	{
// 		// i create a socket
// 		int sockFd = socket(AF_INET, SOCK_STREAM, 0);
// 		if (sockFd < 0)
// 		{
// 			std::cerr << "Socket failed : " << strerror(errno) << std::endl;
// 			continue;
// 		}
// 		int option;
// 		option = 1;
// 		setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)); // with this function i tell the OS
// 		// that is okay to reuse the port even if it is in wait state and all this happend because of this macro SO_REUSEADDR
// 		// i set the socket to the non_blocking mode
// 		if (!setNonBlocking(sockFd))
// 		{
// 			std::cerr << "The processing failed : " << strerror(errno) << " !!!" << std::endl;
// 			close(sockFd);
// 			continue;
// 		}
// 		// configure the struct sockaddr_in
// 		// struct sockaddr_in used to represent the IPV4 socket addresses in the network programming
// 		struct sockaddr_in addr;
// 		memset(&addr, 0, sizeof(addr));
// 		addr.sin_family = AF_INET;								   // set the address family to IPV4
// 		addr.sin_port = htons(servers[i].port);					   // set the port and conver it from host byte order to network byte order
// 		addr.sin_addr.s_addr = inet_addr(servers[i].host.c_str()); // convert IPV4 from string to to network byte, binary format
// 		// bind socket to the address
// 		if (bind(sockFd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
// 		{
// 			std::cerr << "Bind failed on : " << servers[i].host << ":" << servers[i].port << ": " << strerror(errno) << std::endl;
// 			close(sockFd);
// 			continue;
// 		}
// 		// start listening for connection
// 		if (listen(sockFd, 128) < 0) // 128 (the typical OS default) and 511 the nginx default
// 		{
// 			std::cerr << "Listening failed : " << strerror(errno) << std::endl;
// 			close(sockFd);
// 			continue;
// 		}
// 		// store the socket information
// 		std::cout << "Listening on : " << servers[i].host << ":" << servers[i].port << std::endl;
// 		// I store the info in the listenSockets struct
// 		ListeningSocket info;
// 		info.fd = sockFd;
// 		info.host = servers[i].host;
// 		info.port = servers[i].port;
// 		listeningSocket.push_back(info);
// 		// add to poll
// 		struct pollfd pfd;
// 		pfd.fd = sockFd;
// 		pfd.events = POLLIN; // here We are interested in readable events that is the incoming connections
// 		pfd.revents = 0;
// 		pollfds.push_back(pfd);
// 	}
// 	if (listeningSocket.empty())
// 		return (false);
// 	else
// 		return (true);
// };

// void WebServer::run()
// {
// 	mainEventLoop();
// };

// void WebServer::mainEventLoop()
// {
// 	while (true)
// 	{
// 		int res = poll(&pollfds[0], pollfds.size(), 100);
// 		if (res < 0)
// 		{
// 			std::cerr << "Poll failed : " << strerror(errno) << std::endl;
// 			break;
// 		}

// 		size_t n = pollfds.size();
// 		for (size_t i = 0; i < n; ++i)
// 		{
// 			int fd = pollfds[i].fd;
// 			// Check if this is a listening socket
// 			bool isListening = false;
// 			for (size_t j = 0; j < listeningSocket.size(); j++)
// 			{
// 				if (fd == listeningSocket[j].fd)
// 				{
// 					isListening = true;
// 					break;
// 				}
// 			}
// 			if (isListening)
// 			{
// 				if (pollfds[i].revents & POLLIN)
// 					handleNewConnections(i);
// 				continue;
// 			}
// 			// Client socket
// 			if (pollfds[i].revents & POLLIN)
// 			{
// 				// std::cout << "we will handle new connection\n";
// 				handleClientRead(fd);
// 			}

// 			if (pollfds[i].revents & POLLOUT)
// 			{
// 				// std::cout << "RECEIVING||||||||||||\n";
// 				handleClientWrite(fd);
// 			}
// 		}

// 		// Remove disconnected clients here
// 		for (size_t i = 0; i < pollfds.size();)
// 		{
// 			int fd = pollfds[i].fd;
// 			bool isListening = false;
// 			for (size_t j = 0; j < listeningSocket.size(); j++)
// 				if (fd == listeningSocket[j].fd)
// 					isListening = true;
// 			if (!isListening && clients.count(fd) && clients[fd].disconnect)
// 			{
// 				close(fd);
// 				pollfds.erase(pollfds.begin() + i);
// 				clients.erase(fd);
// 			}
// 			else
// 				++i;
// 		}
// 	}
// }

// void WebServer::handleNewConnections(size_t listen_idx)
// {
// 	int listenFd = pollfds[listen_idx].fd;
// 	while (true)
// 	{
// 		struct sockaddr_in client_addr;
// 		socklen_t client_len = sizeof(client_addr);
// 		int client_fd = accept(listenFd, (struct sockaddr *)&client_addr, &client_len);
// 		if (client_fd < 0)
// 		{
// 			if (errno == EAGAIN || errno == EWOULDBLOCK)
// 				break;
// 			std::cerr << "Accept failed : " << strerror(errno) << std::endl;
// 			break;
// 		}
// 		setNonBlocking(client_fd);
// 		std::cout << "Accepted client from : " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;
// 		struct pollfd client_pfd;
// 		client_pfd.fd = client_fd;
// 		client_pfd.events = POLLIN;
// 		client_pfd.revents = 0;
// 		pollfds.push_back(client_pfd);
// 		clients[client_fd] = Client(client_fd);
// 	}
// }

// void WebServer::handleDeleteMethod(int fd)
// {
// 	const ServerBlock *server = findServerBlock(fd);
// 	const LocationBlock *location = NULL;
// 	if (server != NULL)
// 		location = findLocationBlock(*server, clients[fd].request.path);

// 	bool deleteIsAllowed = false;
// 	if (location != NULL)
// 	{
// 		for (size_t i = 0; i < location->allowed_methods.size(); i++)
// 		{
// 			if (location->allowed_methods[i] == "DELETE")
// 			{
// 				deleteIsAllowed = true;
// 				break;
// 			}
// 		}
// 	}
// 	else if (server != NULL)
// 	{
// 		for (size_t i = 0; i < server->directives.size(); i++)
// 		{
// 			if (server->directives[i].name == "allow_methods")
// 			{
// 				for (size_t j = 0; j < server->directives[i].values.size(); j++)
// 				{
// 					if (server->directives[i].values[j] == "DELETE")
// 					{
// 						deleteIsAllowed = true;
// 						break;
// 					}
// 				}
// 			}
// 		}
// 	}
// 	std::string root;
// 	if (location != NULL)
// 		root = location->root;
// 	else if (server != NULL)
// 		root = server->root;
// 	else
// 		root = "./";
// 	std::string filePath = resolveFilePath(root, clients[fd].request.path);

// 	std::string response;
// 	if (!deleteIsAllowed)
// 		response = buildHttpResponse(405, "Method Not Allowed", "Method Not Allowed");
// 	else
// 	{
// 		struct stat statbuffer;
// 		if (stat(filePath.c_str(), &statbuffer) != 0)
// 			response = buildHttpResponse(404, "Not Found", "Not Found");
// 		else if (S_ISDIR(statbuffer.st_mode))
// 			response = buildHttpResponse(403, "Forbidden", "Forbidden");
// 		else
// 		{
// 			if (remove(filePath.c_str()) == 0)
// 				response = buildHttpResponse(200, "OK", "Deleted");
// 			else
// 				response = buildHttpResponse(500, "Internal Server Error", "Internal Server Error");
// 		}
// 	}
// 	clients[fd].responseBuffer = response;
// 	clients[fd].responseDone = false;
// 	for (size_t i = 0; i < pollfds.size(); i++)
// 	{
// 		if (pollfds[i].fd == fd)
// 			pollfds[i].events = POLLOUT;
// 	}
// }

// void WebServer::handleGetMethod(int fd)
// {
// 	const ServerBlock *server = findServerBlock(fd);
// 	if (!server)
// 	{
// 		clients[fd].responseBuffer = "HTTP/1.0 500 Internal Server Error\r\nContent-Length: 21\r\n\r\nServer config error";
// 	}
// 	else
// 	{
// 		const LocationBlock *location = findLocationBlock(*server, clients[fd].request.path);

// 		std::string root;
// 		if (location != NULL)
// 			root = location->root;
// 		else
// 			root = server->root;

// 		std::string filepath = resolveFilePath(root, clients[fd].request.path);

// 		struct stat statbuf;
// 		if (stat(filepath.c_str(), &statbuf) == 0)
// 		{
// 			if (S_ISDIR(statbuf.st_mode))
// 			{
// 				std::string index_file = filepath;
// 				if (index_file.size() > 0)
// 				{
// 					if (index_file[index_file.size() - 1] != '/')
// 						index_file += "/";
// 				}
// 				std::string index_name;
// 				if (location != NULL)
// 					index_name = location->index;
// 				else
// 					index_name = server->index;
// 				if (index_name.empty())
// 					index_name = "index.html";
// 				index_file += index_name;
// 				clients[fd].responseBuffer = buildHttpResponse(index_file, *server);
// 			}
// 			else
// 			{
// 				clients[fd].responseBuffer = buildHttpResponse(filepath, *server);
// 			}
// 		}
// 		else
// 		{
// 			clients[fd].responseBuffer = buildHttpResponse(filepath, *server);
// 		}
// 	}
// 	clients[fd].responseDone = false;
// 	size_t i;
// 	for (i = 0; i < pollfds.size(); ++i)
// 	{
// 		if (pollfds[i].fd == fd)
// 		{
// 			pollfds[i].events = POLLOUT;
// 		}
// 	}
// }

// void WebServer::executeCgiScript(int fd, const ServerBlock *server, const LocationBlock *location)
// {
// 	std::string root = location->root.empty() ? server->root : location->root;
// 	std::string scriptPath = resolveFilePath(root, clients[fd].request.path);

// 	// Check if the script exists and is executable
// 	struct stat statbuf;
// 	if (stat(scriptPath.c_str(), &statbuf) != 0)
// 	{
// 		clients[fd].responseBuffer = buildHttpResponse(404, "Not Found", "CGI script not found");
// 		clients[fd].responseDone = false;
// 		for (size_t i = 0; i < pollfds.size(); i++)
// 		{
// 			if (pollfds[i].fd == fd)
// 				pollfds[i].events = POLLOUT;
// 		}
// 		return;
// 	}

// 	// Create pipes for communication with the CGI script
// 	int inputPipe[2];  // Server to CGI
// 	int outputPipe[2]; // CGI to Server

// 	if (pipe(inputPipe) < 0 || pipe(outputPipe) < 0)
// 	{
// 		clients[fd].responseBuffer = buildHttpResponse(500, "Internal Server Error", "Failed to create pipes");
// 		clients[fd].responseDone = false;
// 		for (size_t i = 0; i < pollfds.size(); i++)
// 		{
// 			if (pollfds[i].fd == fd)
// 				pollfds[i].events = POLLOUT;
// 		}
// 		return;
// 	}

// 	pid_t pid = fork();
// 	if (pid < 0)
// 	{
// 		// Fork failed
// 		close(inputPipe[0]);
// 		close(inputPipe[1]);
// 		close(outputPipe[0]);
// 		close(outputPipe[1]);
// 		clients[fd].responseBuffer = buildHttpResponse(500, "Internal Server Error", "Failed to fork process");
// 		clients[fd].responseDone = false;
// 		for (size_t i = 0; i < pollfds.size(); i++)
// 		{
// 			if (pollfds[i].fd == fd)
// 				pollfds[i].events = POLLOUT;
// 		}
// 		return;
// 	}
// 	else if (pid == 0)
// 	{
// 		// Child process (CGI script)
// 		close(inputPipe[1]);  // Close write end of input pipe
// 		close(outputPipe[0]); // Close read end of output pipe

// 		// Redirect stdin to read from input pipe
// 		dup2(inputPipe[0], STDIN_FILENO);
// 		close(inputPipe[0]);

// 		// Redirect stdout to write to output pipe
// 		dup2(outputPipe[1], STDOUT_FILENO);
// 		close(outputPipe[1]);

// 		// Set up CGI environment variables
// 		std::vector<std::string> env;
// 		env.push_back("GATEWAY_INTERFACE=CGI/1.1");
// 		env.push_back("SERVER_PROTOCOL=HTTP/1.1");
// 		env.push_back("REQUEST_METHOD=" + clients[fd].request.method);
// 		env.push_back("SCRIPT_FILENAME=" +scriptPath);
// 		env.push_back("SCRIPT_NAME=" + clients[fd].request.path);

// 		// Set content information from headers
// 		if (clients[fd].request.headers.count("Content-Length"))
// 			env.push_back("CONTENT_LENGTH=" + clients[fd].request.headers["Content-Length"]);
// 		if (clients[fd].request.headers.count("Content-Type"))
// 			env.push_back("CONTENT_TYPE=" + clients[fd].request.headers["Content-Type"]);

// 		// Set HTTP headers as environment variables (with HTTP_ prefix)
// 		for (std::map<std::string, std::string>::const_iterator it = clients[fd].request.headers.begin(); it != clients[fd].request.headers.end(); ++it)
// 		{
// 			std::string name = "HTTP_" + it->first;
// 			// Replace dashes with underscores and convert to uppercase
// 			for (size_t i = 0; i < name.length(); i++)
// 			{
// 				if (name[i] == '-')
// 					name[i] = '_';
// 				else if (name[i] >= 'a' && name[i] <= 'z')
// 					name[i] = name[i] - 'a' + 'A';
// 			}
// 			env.push_back(name + "=" + it->second);
// 		}
// 		std::vector<char *> envp;
// 		for (size_t i = 0; i < env.size(); i++)
// 			envp.push_back(const_cast<char *>(env[i].c_str()));
// 		envp.push_back(NULL);
// 		// Execute the CGI script
// 		char *const args[] = {(char *)location->cgi_path.c_str(), (char *)scriptPath.c_str(), NULL};
// 		execve(location->cgi_path.c_str(), args, &envp[0]);

// 		// If execve returns, there was an error
// 		exit(1);
// 	}
// 	else
// 	{
// 		// Parent process (server)
// 		close(inputPipe[0]);  // Close read end of input pipe
// 		close(outputPipe[1]); // Close write end of output pipe

// 		// Write request body to the CGI script
// 		if (!clients[fd].request.body.empty())
// 		{
// 			write(inputPipe[1], clients[fd].request.body.c_str(), clients[fd].request.body.length());
// 		}
// 		close(inputPipe[1]); // Close the pipe after writing

// 		// Read CGI output
// 		std::string cgiOutput;
// 		char buffer[4096];
// 		ssize_t bytesRead;

// 		while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0)
// 		{
// 			cgiOutput.append(buffer, bytesRead);
// 		}
// 		close(outputPipe[0]);

// 		// Wait for the CGI process to finish
// 		int status;
// 		waitpid(pid, &status, 0);

// 		// Process CGI output
// 		std::string::size_type headerEnd = cgiOutput.find("\r\n\r\n");
// 		if (headerEnd == std::string::npos)
// 		{
// 			// No headers found, assume it's just body content
// 			std::ostringstream stream;
// 			stream << "HTTP/1.1 200 OK\r\n"
// 				   << "Content-Type: text/html\r\n"
// 				   << "Content-Length: " << cgiOutput.length() << "\r\n"
// 				   << "\r\n"
// 				   << cgiOutput;
// 			clients[fd].requestBuffer = stream.str();
// 		}
// 		else
// 		{
// 			// Headers found, separate them from the body
// 			std::string headers = cgiOutput.substr(0, headerEnd);
// 			std::string body = cgiOutput.substr(headerEnd + 4); // +4 for \r\n\r\n

// 			// Check if headers include Status header
// 			bool hasStatus = false;
// 			std::string status = "200 OK";

// 			std::istringstream headerStream(headers);
// 			std::string headerLine;
// 			while (std::getline(headerStream, headerLine))
// 			{
// 				if (headerLine.substr(0, 7) == "Status:")
// 				{
// 					status = headerLine.substr(8);
// 					hasStatus = true;
// 					break;
// 				}
// 			}

// 			// Construct response
// 			if (hasStatus)
// 			{
// 				clients[fd].responseBuffer = "HTTP/1.1 " + status + "\r\n" + headers + "\r\n\r\n" + body;
// 			}
// 			else
// 			{
// 				clients[fd].responseBuffer = "HTTP/1.1 200 OK\r\n" + headers + "\r\n\r\n" + body;
// 			}
// 		}

// 		clients[fd].responseDone = false;
// 		for (size_t i = 0; i < pollfds.size(); i++)
// 		{
// 			if (pollfds[i].fd == fd)
// 			{
// 				pollfds[i].events = POLLOUT;
// 				break;
// 			}
// 		}
// 	}
// }

// void WebServer::handleFileUpload(int fd, const LocationBlock *location)
// {
// 	std::string contentType = ""; // Extract Content-Type header to check for multipart form data
// 	if (clients[fd].request.headers.count("Content-Type"))
// 		contentType = clients[fd].request.headers["Content-Type"];
// 	// Check if this is a multipart/form-data upload
// 	if (contentType.find("multipart/form-data") == std::string::npos)
// 	{
// 		clients[fd].responseBuffer = buildHttpResponse(400, "Bad Request", "Content-Type must be multipart/form-data for file uploads");
// 		clients[fd].responseDone = false;
// 		return;
// 	}
// 	std::string boundary;
// 	size_t boundaryPos = contentType.find("boundary=");
// 	if (boundaryPos != std::string::npos)
// 		boundary = contentType.substr(boundaryPos + 9);
// 	else
// 	{
// 		clients[fd].responseBuffer = buildHttpResponse(400, "Bad Request", "No boundary in Content-Type header");
// 		clients[fd].responseDone = false;
// 		return;
// 	}
// 	std::string body = clients[fd].request.body;
// 	std::string boundaryMarker = "--" + boundary;
// 	std::string endBoundary = boundaryMarker + "--";
// 	size_t startPos = body.find(boundaryMarker);
// 	if (startPos == std::string::npos)
// 	{
// 		clients[fd].responseBuffer = buildHttpResponse(400, "Bad Request", "Invalid multipart form data");
// 		clients[fd].responseDone = false;
// 		return;
// 	}
// 	size_t filenamePos = body.find("filename=\"", startPos);
// 	if (filenamePos == std::string::npos)
// 	{
// 		clients[fd].responseBuffer = buildHttpResponse(400, "Bad Request", "No filename in upload");
// 		clients[fd].responseDone = false;
// 		return;
// 	}
// 	filenamePos += 10; // Skip "filename=\""
// 	size_t filenameEnd = body.find("\"", filenamePos);
// 	if (filenameEnd == std::string::npos)
// 	{
// 		clients[fd].responseBuffer = buildHttpResponse(400, "Bad Request", "Invalid filename format");
// 		clients[fd].responseDone = false;
// 		return;
// 	}
// 	std::string filename = body.substr(filenamePos, filenameEnd - filenamePos);
// 	size_t contentStart = body.find("\r\n\r\n", filenameEnd);
// 	if (contentStart == std::string::npos)
// 	{
// 		clients[fd].responseBuffer = buildHttpResponse(400, "Bad Request", "Invalid multipart format");
// 		clients[fd].responseDone = false;
// 		return;
// 	}
// 	contentStart += 4; // Skip \r\n\r\n
// 	size_t contentEnd = body.find(boundaryMarker, contentStart);
// 	if (contentEnd == std::string::npos)
// 	{
// 		contentEnd = body.find(endBoundary, contentStart);
// 		if (contentEnd == std::string::npos)
// 		{
// 			clients[fd].responseBuffer = buildHttpResponse(400, "Bad Request", "Could not find end of file content");
// 			clients[fd].responseDone = false;
// 			return;
// 		}
// 	}
// 	std::string fileContent = body.substr(contentStart, contentEnd - contentStart - 2); // The actual file content is between contentStart and contentEnd - 2 (\r\n before boundary)
// 	struct stat st;																		// Create upload directory if it doesn't exist
// 	if (stat(location->upload_path.c_str(), &st) != 0)									// Directory doesn't exist, try to create it
// 	{
// 		if (mkdir(location->upload_path.c_str(), 0755) != 0)
// 		{
// 			clients[fd].responseBuffer = buildHttpResponse(500, "Internal Server Error", "Failed to create upload directory");
// 			clients[fd].responseDone = false;
// 			return;
// 		}
// 	}
// 	std::string filePath = location->upload_path; // Build the full path for the uploaded file
// 	if (filePath[filePath.size() - 1] != '/')
// 		filePath += '/';
// 	filePath += filename;
// 	std::ofstream outFile(filePath.c_str(), std::ios::binary);
// 	if (!outFile)
// 	{
// 		clients[fd].responseBuffer = buildHttpResponse(500, "Internal Server Error", "Failed to open file for writing");
// 		clients[fd].responseDone = false;
// 		return;
// 	}
// 	outFile.write(fileContent.c_str(), fileContent.size());
// 	outFile.close();
// 	// return succes response
// 	std::string succesMessage = "File '" + filename + "' uploaded successfully";
// 	clients[fd].responseBuffer = buildHttpResponse(201, "Created", succesMessage);
// 	clients[fd].responseDone = false;
// 	for (size_t i = 0; i < pollfds.size(); i++)
// 	{
// 		if (pollfds[i].fd == fd)
// 			pollfds[i].events = POLLOUT;
// 	}
// }

// void WebServer::handlePostMethod(int fd)
// {
// 	const ServerBlock *server = findServerBlock(fd);
// 	if (!server)
// 	{
// 		clients[fd].responseBuffer = buildHttpResponse(500, "Internal Server Error", "Server config error");
// 		clients[fd].responseDone = false;
// 		return;
// 	}

// 	const LocationBlock *location = findLocationBlock(*server, clients[fd].request.path);
// 	bool postIsAllowed = false;
// 	if (location != NULL)
// 	{
// 		for (size_t i = 0; i < location->allowed_methods.size(); i++)
// 		{
// 			if (location->allowed_methods[i] == "POST")
// 			{
// 				postIsAllowed = true;
// 				return;
// 			}
// 		}
// 	}
// 	else if (server != NULL)
// 	{
// 		for (size_t i = 0; i < server->directives.size(); i++)
// 		{
// 			if (server->directives[i].name == "allow_method")
// 			{
// 				for (size_t j = 0; j < server->directives[j].values.size(); j++)
// 				{
// 					if (server->directives[i].values[j] == "POST")
// 					{
// 						postIsAllowed = true;
// 						return;
// 					}
// 				}
// 			}
// 		}
// 	}
// 	if (!postIsAllowed)
// 	{
// 		clients[fd].responseBuffer = buildHttpResponse(405, "Method Not Allowed", "Method Not Allowed");
// 		clients[fd].responseDone = false;
// 		for (size_t i = 0; i < pollfds.size(); i++)
// 		{
// 			if (pollfds[i].fd == fd)
// 				pollfds[i].events = POLLOUT;
// 		}
// 		return;
// 	}
// 	bool isCgi = false;
// 	if (location != NULL && !location->cgi_path.empty() && !location->cgi_extention.empty())
// 	{
// 		std::string path = clients[fd].request.path;
// 		for (size_t i = 0; i < location->cgi_extention.size(); i++)
// 		{
// 			std::string ext = location->cgi_extention[i];
// 			if (ext[0] != '.')
// 				ext = "." + ext;

// 			if (path.length() > ext.length() && path.substr(path.length() - ext.length()) == ext)
// 			{
// 				isCgi = true;
// 				break;
// 			}
// 		}
// 	}
// 	if (isCgi)
// 	{
// 		executeCgiScript(fd, server, location);
// 		return;
// 	}
// 	if (location != NULL && !location->upload_path.empty())
// 	{
// 		handleFileUpload(fd, location);
// 		return;
// 	}

// 	clients[fd].responseBuffer = buildHttpResponse(200, "OK", "POST request received but no handler configured");
// 	clients[fd].responseDone = false;
// 	for (size_t i = 0; i < pollfds.size(); i++)
// 	{
// 		if (pollfds[i].fd == fd)
// 			pollfds[i].events = POLLOUT;
// 	}
// }

// void WebServer::handleCGIRequest(int fd)
// {
// 	const ServerBlock *server = findServerBlock(fd);
// 	if (!server)
// 	{
// 		clients[fd].responseBuffer = buildHttpResponse(500, "Internal Server Error", "Server config error");
// 		clients[fd].responseDone = false;
// 		return;
// 	}

// 	const LocationBlock *location = findLocationBlock(*server, clients[fd].request.path);
// 	if (!location || location->cgi_path.empty())
// 	{
// 		clients[fd].responseBuffer = buildHttpResponse(404, "Not Found", "CGI handler not configured");
// 		clients[fd].responseDone = false;
// 		return;
// 	}

// 	std::string root = location->root.empty() ? server->root : location->root;
// 	std::string scriptPath = resolveFilePath(root, clients[fd].request.path);

// 	struct stat statbuf;
// 	if (stat(scriptPath.c_str(), &statbuf) != 0 || !(statbuf.st_mode & S_IXUSR))
// 	{
// 		clients[fd].responseBuffer = buildHttpResponse(404, "Not Found", "CGI script not found or not executable");
// 		clients[fd].responseDone = false;
// 		return;
// 	}

// 	int inputPipe[2];
// 	int outputPipe[2];
// 	if (pipe(inputPipe) < 0 || pipe(outputPipe) < 0)
// 	{
// 		clients[fd].responseBuffer = buildHttpResponse(404, "Internal Server Error", "Failed to create pipes");
// 		clients[fd].responseDone = false;
// 		return;
// 	}

// 	pid_t pid = fork();
// 	if (pid < 0)
// 	{ // fork failed
// 		close(inputPipe[0]);
// 		close(inputPipe[1]);
// 		close(outputPipe[0]);
// 		close(outputPipe[1]);
// 		clients[fd].responseBuffer = buildHttpResponse(500, "Internal Server Error", "Failed to fork process");
// 		clients[fd].responseDone = false;
// 		return;
// 	}
// 	else if (pid == 0)
// 	{ // child process(CGI SCRIPT)
// 		close(inputPipe[1]);
// 		close(outputPipe[0]);
// 		// redirect stdin to read from input pipe
// 		dup2(inputPipe[0], STDIN_FILENO);
// 		close(inputPipe[0]);
// 		// redirect stdout to write to output pipe
// 		dup2(outputPipe[1], STDOUT_FILENO);
// 		close(outputPipe[1]);
// 		// set up CGI environmet variables
// 		std::vector<std::string> env;
// 		env.push_back("GATEWAY_INTERFACE=CGI/1.1");
// 		env.push_back("SERVER_PROTOCOL=HTTP/1.1");
// 		env.push_back("REQUEST_METHOD=" + clients[fd].request.method);
// 		env.push_back("SCRIPT_FILENAME=" + scriptPath);
// 		env.push_back("SCRIPT_NAME=" + clients[fd].request.path);
// 		env.push_back("QUERY_STRING=");
// 		if (clients[fd].request.headers.count("Content-Length"))
// 			env.push_back("Content-Length=" + clients[fd].request.headers["Content-Length"]);
// 		if (clients[fd].request.headers.count("Content-Type"))
// 			env.push_back("Content-Type=" + clients[fd].request.headers["Content-Type"]);
// 		// execute the CGI script
// 		std::vector<char *> envp;
// 		for (size_t i = 0; i < env.size(); i++)
// 			envp.push_back(const_cast<char *>(env[i].c_str()));
// 		envp.push_back(NULL);
// 		char *const args[] = {(char *)location->cgi_path.c_str(), (char *)scriptPath.c_str(), NULL};
// 		execve(location->cgi_path.c_str(), args, &envp[0]);
// 		exit(1);
// 	}
// 	else
// 	{
// 		close(inputPipe[0]);
// 		close(outputPipe[1]);
// 		if (!clients[fd].request.body.empty())
// 			write(inputPipe[1], clients[fd].request.body.c_str(), clients[fd].request.body.length());
// 		close(inputPipe[1]);
// 		std::string cgiOutput;
// 		char buffer[4096];
// 		ssize_t byteRead;
// 		while ((byteRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0)
// 			cgiOutput.append(buffer, byteRead);
// 		close(outputPipe[0]);

// 		int status;
// 		waitpid(pid, &status, 0);

// 		std::string::size_type headerEnd = cgiOutput.find("\r\n\r\n");
// 		if (headerEnd == std::string::npos)
// 		{
// 			std::ostringstream responseStream;
// 			responseStream << "HTTP/1.1 200 OK\r\n"
// 						   << "Content-Type: text/html\r\n"
// 						   << "Content-Length: " << cgiOutput.length() << "\r\n"
// 						   << "\r\n"
// 						   << cgiOutput;
// 			clients[fd].responseBuffer = responseStream.str();
// 		}
// 		else
// 		{ // Headers found, separate them from the body
// 			std::string headers = cgiOutput.substr(0, headerEnd);
// 			std::string body = cgiOutput.substr(headerEnd + 4);
// 			// Check if headers include Status header
// 			bool hasStatus = false;
// 			if (headers.find("Status:") != std::string::npos)
// 				hasStatus = true;

// 			if (hasStatus)
// 				clients[fd].responseBuffer = "HTTP/1.1 " + headers + "\r\n\r\n" + body;
// 			else
// 				clients[fd].responseBuffer = "HTTP/1.1 200 OK\r\n" + headers + "\r\n\r\n" + body;
// 		}
// 		clients[fd].responseDone = false;
// 	}
// 	for (size_t i = 0; i < pollfds.size(); i++)
// 	{
// 		if (pollfds[i].fd == fd)
// 		{
// 			pollfds[i].events = POLLOUT;
// 			break;
// 		}
// 	}
// }

// bool WebServer::isCGIRequest(const std::string &path, const LocationBlock *location)
// {
// 	if (!location || location->cgi_path.empty() || location->cgi_extention.empty())
// 		return (false);
	
// 	for (size_t i = 0; i < location->cgi_extention.size(); i++)
// 	{
// 		std::string ext = location->cgi_extention[i];
// 		if (ext[0] != '.')
// 			ext = "." + ext;
// 		if (path.length() > ext.length() && path.substr(path.length() - ext.length()) == ext)
// 			return (true);
// 	}
// 	return (false);
// }

// void WebServer::handleClientRead(int fd)
// {
// 	char buf[1024];
// 	ssize_t n = recv(fd, buf, sizeof(buf), 0);
// 	if (n <= 0)
// 	{
// 		clients[fd].disconnect = true;
// 		return;
// 	}
// 	clients[fd].requestBuffer.append(buf, n);
// 	if (clients[fd].requestBuffer.find("\r\n\r\n") != std::string::npos)
// 	{
// 		if (clients[fd].request.parser(clients[fd].requestBuffer))
// 		{
// 			const ServerBlock *server = findServerBlock(fd); // when i debuged this it return all its members
// 			const LocationBlock *location = NULL;
// 			if (server != NULL)
// 				location = findLocationBlock(*server, clients[fd].request.path); //it returned all data members after modification
// 			// std::cout << "THERE AREE LOCATION INFO  :\n";
// 			// std::cout << "location path is : " << location->path << std::endl;
// 			// std::cout << "location directives are : " << std::endl;
// 			// // Loop through all directives
// 			// for (size_t i = 0; i < location->directives.size(); ++i)
// 			// {
// 			// 	// Access directive name
// 			// 	std::string directiveName = location->directives[i].name;

// 			// 	// Access directive values
// 			// 	for (size_t j = 0; j < location->directives[i].values.size(); ++j)
// 			// 	{
// 			// 		std::string value = location->directives[i].values[j];
// 			// 		std::cout << "Directive: " << directiveName
// 			// 				  << " Value: " << value << std::endl;
// 			// 	}
// 			// }
// 			// std::cout << "location root is : " << location->root << std::endl;
// 			// std::cout << "location index is : " << location->index << std::endl;
// 			// std::cout << "allow methods :\n";
// 			// for (size_t i = 0; i < location->allowed_methods.size(); i++)
// 			// {
// 			// 	std::cout << "allocation method is : " << location->allowed_methods[i] << std::endl;
// 			// }
// 			// std::cout << "Location autoindex 0 if false : " << location->autoindex << std::endl;
// 			// std::cout << "location redirect url : " << location->redirect_url << std::endl;
// 			// std::cout << "location cgi path is : " << location->cgi_path << std::endl;
// 			// std::cout << "location upload file is : " << location->upload_path << std::endl;
// 			// std::cout << "location cgi extentions:\n";
// 			// for (size_t i = 0; i < location->cgi_extention.size(); i++)
// 			// 	std::cout << location->cgi_extention[i] << std::endl;
// 			if (isCGIRequest(clients[fd].request.path, location))
// 			{
// 				std::cout << "HERE WE SHOULD BE\\\\\\\\\n";
// 				handleCGIRequest(fd);
// 				return;
// 			}
// 			if (clients[fd].request.method == "DELETE")
// 			{
// 				std::cout << "WE ARE IN DELETE METHOD\n";
// 				handleDeleteMethod(fd);
// 				return;
// 			}
// 			else if (clients[fd].request.method == "GET")
// 			{
// 				std::cout << "WE ARE IN GET METHOD\n";
// 				handleGetMethod(fd);
// 				return;
// 			}
// 			else if (clients[fd].request.method == "POST")
// 			{
// 				std::cout << "WE ARE IN POST METHOD\n";
// 				handlePostMethod(fd);
// 				return;
// 			}
// 			else
// 			{
// 				clients[fd].responseBuffer = "HTTP/1.0 405 Method Not Allowed\r\nContent-Length: 18\r\n\r\nMethod Not Allowed";
// 				clients[fd].responseDone = false;
// 				size_t i;
// 				for (i = 0; i < pollfds.size(); ++i)
// 				{
// 					if (pollfds[i].fd == fd)
// 						pollfds[i].events = POLLOUT;
// 				}
// 			}
// 		}
// 	}
// };

// void WebServer::handleClientWrite(int fd)
// {
// 	if (clients[fd].responseDone)
// 		return;
// 	std::cerr << "Sending response to fd: " << fd << std::endl;
// 	std::cerr << "Response length: " << clients[fd].responseBuffer.size() << std::endl;

// 	ssize_t n = send(fd, clients[fd].responseBuffer.c_str(), clients[fd].responseBuffer.size(), 0);
// 	if (n <= 0)
// 	{
// 		clients[fd].disconnect = true;
// 		return;
// 	}
// 	clients[fd].responseBuffer.erase(0, n);
// 	if (clients[fd].responseBuffer.empty())
// 	{
// 		clients[fd].responseDone = true;
// 		clients[fd].disconnect = true;
// 	}
// }

// const ServerBlock *WebServer::findServerBlock(int fd)
// {
// 	// This function is for helping me to find wich serverblock is used for this connection
// 	size_t i;
// 	size_t j;
// 	for (i = 0; i < listeningSocket.size(); i++)
// 	{
// 		if (listeningSocket[i].fd == fd)
// 		{
// 			int port = listeningSocket[i].port;
// 			for (j = 0; j < server_block.size(); j++)
// 			{
// 				if (server_block[j].port == port)
// 					return (&server_block[j]);
// 			}
// 		}
// 	}
// 	if (server_block.empty())
// 		return (NULL);
// 	else
// 		return (&server_block[0]);
// };

// const LocationBlock *WebServer::findLocationBlock(const ServerBlock &server, const std::string &path) //updated by anas EL MEJDKI
// {
// 	size_t bestLen = 0;
// 	const LocationBlock *bestMatch = NULL;
// 	for (size_t i = 0; i < server.locationBlocks.size(); i++)
// 	{
// 		const LocationBlock& loc = server.locationBlocks[i];
// 		if (path.find(loc.path) == 0)
// 		{
// 			size_t len = loc.path.length();
// 			if (len > bestLen && (len == path.length() || path[len] == '/' || len == 1))
// 			{
// 				bestLen = len;
// 				bestMatch = &loc;
// 			}
// 		}
// 	}
// 	return (bestMatch);
// };

// std::string WebServer::resolveFilePath(const std::string &root, const std::string &url_path)
// {
// 	// This function is for combine root and URL path
// 	std::string path = root;
// 	if (!path.empty() && path[path.size() - 1] != '/')
// 		path += "/";
// 	if (!url_path.empty() && url_path[0] == '/')
// 		path += url_path.substr(1);
// 	else
// 		path += url_path;
// 	return (path);
// };

// std::string WebServer::getContentType(const std::string &path)
// {
// 	if (path.size() >= 5 && path.substr(path.size() - 5) == ".html")
// 		return ("Content-Type: text/html");
// 	if (path.size() >= 4 && path.substr(path.size() - 4) == ".css")
// 		return ("Content-Type: text/css");
// 	if (path.size() >= 3 && path.substr(path.size() - 3) == ".js")
// 		return ("Content-Type: application/javascript");
// 	if (path.size() >= 4 && path.substr(path.size() - 4) == ".png")
// 		return ("Content-Type: image/png");
// 	return ("Content-Type: application/octet-stream");
// }

// std::string WebServer::buildHttpResponse(const std::string &filepath, const ServerBlock &server)
// {
// 	struct stat statbuf;
// 	try
// 	{
// 		// I Check if the file exists
// 		if (stat(filepath.c_str(), &statbuf) != 0)
// 		{
// 			std::map<int, std::string>::const_iterator it = server.error_pages.find(404);
// 			if (it != server.error_pages.end())
// 			{
// 				std::string error_path = resolveFilePath(server.root, it->second);
// 				std::ifstream errfile(error_path.c_str());
// 				if (errfile)
// 				{
// 					std::string body((std::istreambuf_iterator<char>(errfile)), std::istreambuf_iterator<char>());
// 					std::ostringstream resp;
// 					resp << "HTTP/1.0 404 Not Found\r\n";
// 					resp << "Content-Length: " << body.size() << "\r\n";
// 					resp << getContentType(filepath) << "\r\n";
// 					resp << "Connection: close\r\n\r\n";
// 					resp << body;
// 					return resp.str();
// 				}
// 			}
// 			return "HTTP/1.0 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found";
// 		}

// 		if (S_ISDIR(statbuf.st_mode))
// 		{
// 			std::string index_file = filepath;
// 			if (index_file.size() > 0 && index_file[index_file.size() - 1] != '/')
// 				index_file += "/";
// 			std::string index_name;
// 			if (server.index.empty())
// 				index_name = "index.html";
// 			else
// 				index_name = server.index;
// 			index_file += index_name;

// 			std::ifstream idxfile(index_file.c_str());
// 			if (!idxfile)
// 			{
// 				std::map<int, std::string>::const_iterator it = server.error_pages.find(403);
// 				if (it != server.error_pages.end())
// 				{
// 					std::string error_path = resolveFilePath(server.root, it->second);
// 					std::ifstream errfile(error_path.c_str());
// 					if (errfile)
// 					{
// 						std::string body((std::istreambuf_iterator<char>(errfile)), std::istreambuf_iterator<char>());
// 						std::ostringstream resp;
// 						resp << "HTTP/1.0 403 Forbidden\r\n";
// 						resp << "Content-Length: " << body.size() << "\r\n";
// 						resp << getContentType(filepath) << "\r\n";
// 						resp << "Connection: close\r\n\r\n";
// 						resp << body;
// 						return resp.str();
// 					}
// 				}
// 				return "HTTP/1.0 403 Forbidden\r\nContent-Length: 9\r\n\r\nForbidden";
// 			}

// 			std::string body((std::istreambuf_iterator<char>(idxfile)), std::istreambuf_iterator<char>());
// 			std::ostringstream resp;
// 			resp << "HTTP/1.0 200 OK\r\n";
// 			resp << "Content-Length: " << body.size() << "\r\n";
// 			resp << getContentType(filepath) << "\r\n";
// 			resp << "Connection: close\r\n\r\n";
// 			resp << body;
// 			return resp.str();
// 		}
// 		else
// 		{
// 			std::ifstream file(filepath.c_str());
// 			if (!file)
// 			{
// 				std::map<int, std::string>::const_iterator it = server.error_pages.find(404);
// 				if (it != server.error_pages.end())
// 				{
// 					std::string error_path = resolveFilePath(server.root, it->second);
// 					std::ifstream errfile(error_path.c_str());
// 					if (errfile)
// 					{
// 						std::string body((std::istreambuf_iterator<char>(errfile)), std::istreambuf_iterator<char>());
// 						std::ostringstream resp;
// 						resp << "HTTP/1.0 404 Not Found\r\n";
// 						resp << "Content-Length: " << body.size() << "\r\n";
// 						resp << getContentType(filepath) << "\r\n";
// 						resp << "Connection: close\r\n\r\n";
// 						resp << body;
// 						return resp.str();
// 					}
// 				}
// 				return "HTTP/1.0 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found";
// 			}

// 			std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
// 			std::ostringstream resp;
// 			resp << "HTTP/1.0 200 OK\r\n";
// 			resp << "Content-Length: " << body.size() << "\r\n";
// 			resp << getContentType(filepath) << "\r\n";
// 			resp << "Connection: close\r\n\r\n";
// 			resp << body;
// 			return resp.str();
// 		}
// 	}
// 	catch (...) // here i catch all exceptions
// 	{
// 		std::map<int, std::string>::const_iterator it = server.error_pages.find(500);
// 		if (it != server.error_pages.end())
// 		{
// 			std::string error_path = resolveFilePath(server.root, it->second);
// 			std::ifstream errfile(error_path.c_str());
// 			if (errfile)
// 			{
// 				std::string body((std::istreambuf_iterator<char>(errfile)), std::istreambuf_iterator<char>());
// 				std::ostringstream resp;
// 				resp << "HTTP/1.0 500 Internal Server Error\r\n";
// 				resp << "Content-Length: " << body.size() << "\r\n";
// 				resp << getContentType(filepath) << "\r\n";
// 				resp << "Connection: close\r\n\r\n";
// 				resp << body;
// 				return resp.str();
// 			}
// 		}
// 		return "HTTP/1.0 500 Internal Server Error\r\nContent-Length: 21\r\n\r\nInternal Server Error";
// 	}
// }

// std::string WebServer::buildHttpResponse(int code, const std::string &status, const std::string &body)
// {
// 	std::ostringstream resp;
// 	resp << "HTTP/1.0 " << code << " " << status << "\r\n";
// 	resp << "Content-Length: " << body.size() << "\r\n";
// 	resp << "Content-Type: text/plain\r\n";
// 	resp << "Connection: close\r\n\r\n";
// 	resp << body;
// 	return resp.str();
// };

// void WebServer::closeSockets()
// {
// 	for (size_t i = 0; i < pollfds.size(); i++)
// 		close(pollfds[i].fd);
// 	pollfds.clear();
// 	listeningSocket.clear();
// 	clients.clear();
// }