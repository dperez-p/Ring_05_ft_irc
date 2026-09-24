/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dperez-p <dperez-p@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 12:11:34 by dperez-p          #+#    #+#             */
/*   Updated: 2026/09/24 13:49:53 by dperez-p         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"

static bool validPort(const std::string &port)
{
	if (port.empty()) // check if the port is empty
	{
		return false;
	}
	bool isAllDigits = (port.find_first_not_of("0123456789") == std::string::npos); //Check if all characters in the string are numeric digits

	if (!isAllDigits) // check if is false
	{
		return false;
	}
	int portNum = std::atoi(port.c_str()); // Convert the C-style string pointer to an integer

	return (portNum >= 1024 && portNum <= 65535); // check if the number are between a range (true) or not (false)
}

int	main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cout << "Usage: " << av[0] << " <port number> <password>" << std::endl;
		return 1;
	}
	Server	ser;
	std::cout << "------------ SERVER ------------" << std::endl;
	try
	{
		signal(SIGINT, Server::signalHandler); //catch the signal (ctrl + C)
		signal(SIGQUIT, Server::signalHandler); //catch the signal (ctrl + \)
		if (!validPort(av[1]) || !*av[2] || std::strlen(av[2]) > 16)
		{
			std::cout << "Invalid port number / Password." << std::endl;
			return 1;
		}
		ser.serverInit(std::atoi(av[1]), av[2]); //initialize the server
	}
	catch(const std::exception& e)
	{
		ser.closeFds();
		std::cerr << e.what() << std::endl;
		std::cout << "The Server Closed!" << std::endl;
		return 1;
	}
	std::cout << "The Server Closed!" << std::endl;
}
