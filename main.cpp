/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aahlaqqa <aahlaqqa@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 17:50:13 by aahlaqqa          #+#    #+#             */
/*   Updated: 2025/11/13 20:12:35 by aibn-ich         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parsing/parser.hpp"
#include "Parsing/configProcessor.hpp"
#include "WebServ/Exposed_Functions.hpp"

int main(int argc, char **argv)
{
    try
    {
        Parser parser;
        ConfigProcessor processor;

        parser.Tokenization(argc, argv);
        parser.parsConfig();
        parser.validatConfig();

        std::vector<ServerBlock> servers_blocks = parser.getServerBlock();
        processor.processServerBlock(servers_blocks);

        API::ServerBlock_Parser(servers_blocks);
        if (1 == API::Webserver())
            return 1;
    }
    catch (const std::exception &e) {
        std::cout << "Error : " << e.what() << std::endl;
    }
    return 0;
}
