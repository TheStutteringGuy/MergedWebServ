// /* ************************************************************************** */
// /*                                                                            */
// /*                                                        :::      ::::::::   */
// /*   Request.cpp                                        :+:      :+:    :+:   */
// /*                                                    +:+ +:+         +:+     */
// /*   By: aahlaqqa <aahlaqqa@student.42.fr>          +#+  +:+       +#+        */
// /*                                                +#+#+#+#+#+   +#+           */
// /*   Created: 2025/07/17 17:06:28 by ahmed             #+#    #+#             */
// /*   Updated: 2025/07/27 09:59:48 by aahlaqqa         ###   ########.fr       */
// /*                                                                            */
// /* ************************************************************************** */

// #include "Request.hpp"

// Request::Request() {};

// Request::~Request() {};

// void Request::clear()
// {
//     method.clear();
//     body.clear();
//     version.clear();
//     path.clear();
//     headers.clear();
// };

// bool Request::parser(const std::string &str)
// {
//     clear();
//     std::istringstream strm(str.c_str());
//     std::string line;

//     // I start the parsing request here
//     if (!std::getline(strm, line))
//         return (false);
//     if (!line.empty() && line[line.length() - 1] == '\r')
//         line.erase(line.length() - 1);
//     std::istringstream reqline(line.c_str());
//     if (!(reqline >> method >> path >> version))
//         return (false);
//     // Here i start parsing the headers
//     while (std::getline(strm, line))
//     {
//         if (!line.empty() && line[line.length() - 1] == '\r')
//             line.erase(line.length() - 1);
//         if (line.empty())
//             break; // End of line
//         size_t pos;
//         pos = line.find(":");
//         if (pos == std::string::npos)
//             continue;
//         std::string key = line.substr(0, pos);
//         std::string value = line.substr(pos + 1);
//         while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
//             value.erase(0, 1);
//         headers[key] = value;
//     }
//     // Case of parsing the body
//     std::string bodyStr;
//     while (std::getline(strm, line))
//         bodyStr += line + "\n";
//     if (!bodyStr.empty() && bodyStr[bodyStr.length() - 1] == '\n')
//         bodyStr.erase(bodyStr.length() - 1);;
//     body = bodyStr;
//     return (true);
// };