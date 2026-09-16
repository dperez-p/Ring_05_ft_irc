/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dperez-p <dperez-p@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 12:11:34 by dperez-p          #+#    #+#             */
/*   Updated: 2026/08/10 12:15:26 by dperez-p         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"

int	main()
{
	Server	ser;
	std::cout << "------------ SERVER ------------" << std::endl;
	try
	{
		signal(SIGINT, Server::signalHandler); //catch the signal (ctrl + C)
		signal(SIGQUIT, Server::signalHandler); //catch the signal (ctrl + \)
		ser.serverInit(); //initialize the server
	}
	catch(const std::exception& e)
	{
		ser.closeFds();
		std::cerr << e.what() << std::endl;
	}
	std::cout << "The Server Closed!" << std::endl;
}
