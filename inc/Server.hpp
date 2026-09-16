/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dperez-p <dperez-p@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 10:54:49 by dperez-p          #+#    #+#             */
/*   Updated: 2026/09/16 11:30:02 by dperez-p         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include <iostream>
#include <vector>
#include <sys/socket.h> //socket()
#include <sys/types.h> //for socket() too
#include <netinet/in.h> // sockaddr_in()
#include <fcntl.h> // for fcntl()
#include <arpa/inet.h> // for inet_ntoa()
#include <poll.h> // for poll()
#include <csignal> //for signal()
#include <cstring>

class Server
{
	private:
		int	_port; // server port
		int	_serSocketFd; // server socket file descriptor
		static bool	_signal; // boolean for signal, static to create one for the class and not for each object
		std::vector<Client> _clients; // vector of clients
		std::vector<struct pollfd> _fds; // vector of pollfd

	public:
		Server();

		void	serverInit(); // server initialization
		void	serSocket(); // server socket creation
		void	acceptNewClient(); // accept new client
		void	recieveNewData(int fd); // recieve new data from a registered client

		static void signalHandler(int signum); // signal handler

		void	closeFds(); // close file descriptors
		void	clearClients(int fd); // clear clients
};
