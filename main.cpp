/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aahlaqqa <aahlaqqa@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 17:50:13 by aahlaqqa          #+#    #+#             */
/*   Updated: 2025/08/01 19:40:25 by aahlaqqa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parsing/parser.hpp"
#include "Parsing/configProcessor.hpp"
#include "Team_Webserver/webServer.hpp"
#include "~TheStutteringGuy_WebServ/Exposed_Functions.hpp"

int main(int argc, char **argv)
{
    try
    {
        Parser parser;
        ConfigProcessor processor;

        parser.Tokenization(argc, argv);
        parser.parsConfig();
        parser.validatConfig();

        parser.printConfig();
        std::vector<ServerBlock> servers_blocks = parser.getServerBlock();
        processor.processServerBlock(servers_blocks);

        API::ServerBlock_Parser(servers_blocks);
        if (1 == API::Webserver())
            return 1;
    }
    catch (const std::exception &e)
    {
        std::cout << "Error : " << e.what() << std::endl;
    }
    return 0;
}
