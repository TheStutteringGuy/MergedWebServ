// #include "webServer.hpp"

// std::string getCGIInterpreter(const std::string &scriptPath, const LocationBlock *location)
// {
//     std::string ext = scriptPath.substr(scriptPath.find_last_of('.'));

//     if (ext == ".php")
//         return ("/usr/bin/php");
//     else if (ext == ".sh")
//         return ("/bin/bash");
//     else if (ext == ".py")
//         return ("/usr/bin/python3");
//     else if (location && !location->cgi_path.empty())
//         return (location->cgi_path);
//     return ("");
// }

// void setupCGIEnvironment(const Client &client, const std::string &scriptPath, const LocationBlock *location, const ServerBlock *server)
// {
//     // static std::vector;
//     setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
//     setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
//     setenv("REQUEST_METHOD", client.request.method.c_str(), 1);
//     setenv("SCRIPT_FILENAME", scriptPath.c_str(), 1);
//     setenv("SCRIPT_NAME", client.request.path.c_str(), 1);
// }