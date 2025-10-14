#pragma once

// System includes
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>


// Project constants
#define DEFAULT_PORT 8080
#define MAX_CONNECTIONS 1000
#define BUFFER_SIZE 4096
#define MAX_REQUEST_SIZE 1000000  // 1MB

// HTTP Status Codes
#define HTTP_OK 200
#define HTTP_NOT_FOUND 404
#define HTTP_INTERNAL_ERROR 500
#define HTTP_BAD_REQUEST 400
#define HTTP_METHOD_NOT_ALLOWED 405

// Color codes for logging
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_RESET "\033[0m"