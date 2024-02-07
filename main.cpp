/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-maar <ael-maar@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/23 11:34:44 by ael-maar          #+#    #+#             */
/*   Updated: 2024/02/06 19:32:41 by ael-maar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
            throw std::string("./webserv filename");
        std::vector<Server> conFile = ServerFill(argv[1]);
        ServerManager serverManager(conFile);

        serverManager.setupServers();
        serverManager.run();
    }
    catch(std::string const &errMessage)
    {
        std::cerr << errMessage << '\n';
    }


    return (EXIT_SUCCESS);
}
