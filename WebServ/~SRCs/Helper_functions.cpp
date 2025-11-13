#include "WebServer.hpp"

void Print_generic_Rules(void)
{
    std::cout << "\n";
    std::cout << "==========================================================\n";
    std::cout << "                    WebServ HTTP Server                   \n";
    std::cout << "==========================================================\n\n";
    
    std::cout << "USAGE:\n";
    std::cout << "  ./webserv [configuration_file]\n\n";
    
    std::cout << "DESCRIPTION:\n";
    std::cout << "  A lightweight HTTP/1.0 web server implementation.\n";
    std::cout << "  If no configuration file is provided, all the blame on you\n\n";
    
    std::cout << "CONFIGURATION FILE:\n";
    std::cout << "  The configuration file defines server behavior including:\n";
    std::cout << "    - Server Ip ports\n";
    std::cout << "    - Document root directories\n";
    std::cout << "    - Allowed HTTP methods (GET, POST, DELETE)\n";
    std::cout << "    - Error page locations\n";
    std::cout << "    - Directory auto-indexing\n";
    std::cout << "    - URL redirections\n\n";
    
    std::cout << "IMPORTANT NOTICE:\n";
    std::cout << "  These are important instructions that rules the webserv.\n";
    std::cout << "  Neglecting any of this will result in The WebServer not working\n";
    std::cout << "    - .Cache directory must be always present in TheServers root_Directory.\n\n";
    
    std::cout << "EXIT:\n";
    std::cout << "  Press Ctrl+C to stop the server.\n\n";
    
    std::cout << "==========================================================\n\n";
}

std::string generic_error_page(unsigned int error_code, std::string phrase)
{
    std::stringstream ss;

    std::string s = " ";
    std::string end = "\n"; 

    ss << "<!DOCTYPE html>" << end;
    ss << "<html lang=\"en\">" << end;
    ss << "<head>" << end;
    ss << "    <meta charset=\"UTF-8\">" << end;
    ss << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" << end;
    ss << "    <title> " << error_code << s << phrase << "</title>" << end;
    ss << "    <style>" << end;
    ss << "        body {" << end;
    ss << "            margin: 0;" << end;
    ss << "            padding: 0;" << end;
    ss << "            background-color: #000000;" << end;
    ss << "            color: #ffffff;" << end;
    ss << "            font-family: 'Courier New', monospace;" << end;
    ss << "            display: flex;" << end;
    ss << "            justify-content: center;" << end;
    ss << "            align-items: center;" << end;
    ss << "            height: 100vh;" << end;
    ss << "        }" << end;
    ss << "        .error-box {" << end;
    ss << "            width: 400px;" << end;
    ss << "            height: 300px;" << end;
    ss << "            border: 2px solid #ffffff;" << end;
    ss << "            display: flex;" << end;
    ss << "            flex-direction: column;" << end;
    ss << "            justify-content: center;" << end;
    ss << "            align-items: center;" << end;
    ss << "            text-align: center;" << end;
    ss << "        }" << end;
    ss << "        .error-code {" << end;
    ss << "            font-size: 72px;" << end;
    ss << "            font-weight: bold;" << end;
    ss << "            margin-bottom: 20px;" << end;
    ss << "        }" << end;
    ss << "        .error-message {" << end;
    ss << "            font-size: 24px;" << end;
    ss << "            margin-bottom: 10px;" << end;
    ss << "        }" << end;
    ss << "        .error-description {" << end;
    ss << "            font-size: 16px;" << end;
    ss << "            color: #cccccc;" << end;
    ss << "        }" << end;
    ss << "    </style>" << end;
    ss << "</head>" << end;
    ss << "<body>" << end;
    ss << "    <div class=\"error-box\">" << end;
    ss << "        <div class=\"error-code\">" << error_code << "</div>" << end;
    ss << "        <div class=\"error-message\">" << phrase << "</div>" << end;
    ss << "        <div class=\"error-description\">The request cannot be processed</div>" << end;
    ss << "    </div>" << end;
    ss << "</body>" << end;
    ss << "</html>";

    return ss.str();
}

std::string generic_index_page(std::string URI_)
{
    std::stringstream ss;

    std::string s = " ";
    std::string end = "\n"; 

    ss << "<!DOCTYPE html>" + end;
    ss << "<html lang=\"en\">" + end;
    ss << "<head>" + end;
    ss << "    <meta charset=\"UTF-8\">" + end;
    ss << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" + end;
    ss << "    <title>Soul Society Directory</title>" + end;
    ss << "    <style>" + end;
    ss << "        body {" + end;
    ss << "            margin: 0;" + end;
    ss << "            padding: 0;" + end;
    ss << "            background-color: #000000;" + end;
    ss << "            color: #ffffff;" + end;
    ss << "            font-family: 'Courier New', monospace;" + end;
    ss << "            display: flex;" + end;
    ss << "            justify-content: center;" + end;
    ss << "            align-items: center;" + end;
    ss << "            min-height: 100vh;" + end;
    ss << "        }" + end;
    ss << "        " + end;
    ss << "        .directory-box {" + end;
    ss << "            width: 500px;" + end;
    ss << "            max-height: 80vh;" + end;
    ss << "            border: 2px solid #ffffff;" + end;
    ss << "            padding: 30px;" + end;
    ss << "            text-align: left;" + end;
    ss << "            overflow-y: auto;" + end;
    ss << "            overflow-x: hidden;" + end;
    ss << "        }" + end;
    ss << "        " + end;
    ss << "        /* Custom scrollbar for the directory box */" + end;
    ss << "        .directory-box::-webkit-scrollbar {" + end;
    ss << "            width: 8px;" + end;
    ss << "        }" + end;
    ss << "        " + end;
    ss << "        .directory-box::-webkit-scrollbar-track {" + end;
    ss << "            background: #000000;" + end;
    ss << "        }" + end;
    ss << "        " + end;
    ss << "        .directory-box::-webkit-scrollbar-thumb {" + end;
    ss << "            background: #ffffff;" + end;
    ss << "            border-radius: 4px;" + end;
    ss << "        }" + end;
    ss << "        " + end;
    ss << "        .directory-box::-webkit-scrollbar-thumb:hover {" + end;
    ss << "            background: #cccccc;" + end;
    ss << "        }" + end;
    ss << "        " + end;
    ss << "        h1 {" + end;
    ss << "            font-size: 24px;" + end;
    ss << "            font-weight: bold;" + end;
    ss << "            margin: 0 0 30px 0;" + end;
    ss << "            text-align: center;" + end;
    ss << "        }" + end;
    ss << "        " + end;
    ss << "        .directory-list {" + end;
    ss << "            list-style: none;" + end;
    ss << "            padding: 0;" + end;
    ss << "            margin: 0;" + end;
    ss << "        }" + end;
    ss << "        " + end;
    ss << "        .directory-list li {" + end;
    ss << "            margin: 8px 0;" + end;
    ss << "        }" + end;
    ss << "        " + end;
    ss << "        .directory-list a {" + end;
    ss << "            color: #ffffff;" + end;
    ss << "            text-decoration: none;" + end;
    ss << "            display: inline-block;" + end;
    ss << "            padding: 5px 0;" + end;
    ss << "            font-size: 16px;" + end;
    ss << "            transition: color 0.2s;" + end;
    ss << "        }" + end;
    ss << "        " + end;
    ss << "        .directory-list a:hover {" + end;
    ss << "            color: #cccccc;" + end;
    ss << "            text-decoration: underline;" + end;
    ss << "        }" + end;
    ss << "        " + end;
    ss << "        .directory-list a:before {" + end;
    ss << "            content: \"📁 \";" + end;
    ss << "            margin-right: 8px;" + end;
    ss << "        }" + end;
    ss << "        " + end;
    ss << "        /* For file links */" + end;
    ss << "        .file-link:before {" + end;
    ss << "            content: \"📄 \" !important;" + end;
    ss << "        }" + end;
    ss << "    </style>" + end;
    ss << "</head>" + end;
    ss << "<body>" + end;
    ss << "    <div class=\"directory-box\">" + end;
    ss << "        <h1>INDEX OF "<< URI_ << "</h1>" + end;
    ss << "        " + end;
    ss << "        <ul class=\"directory-list\">" + end;
    ss << "INSERT HERE" + end;
    ss << "        </ul>" + end;
    ss << "    </div>" + end;
    ss << "</body>" + end;
    ss << "</html>";

    return ss.str();
}