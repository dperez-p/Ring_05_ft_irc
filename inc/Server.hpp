/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lanton-m <lanton-m@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 10:54:49 by dperez-p          #+#    #+#             */
/*   Updated: 2026/09/27 21:50:42 by lanton-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <sys/socket.h> //socket()
#include <sys/types.h> //for socket() too
#include <netinet/in.h> // sockaddr_in()
#include <fcntl.h> // for fcntl()
#include <arpa/inet.h> // for inet_ntoa()
#include <poll.h> // for poll()
#include <csignal> //for signal()
#include <cstring>
#include <unistd.h>
#include <cstdlib>



class Client;
class Channel;
class Message;


class Server
{
	private:
		int	_port; // server port
		int	_serSocketFd; // server socket file descriptor
		static bool	_signal; // boolean for signal, static to create one for the class and not for each object
		std::vector<Client> _clients; // vector of clients
		std::vector<Channel> _channel; // vector of channels
		std::vector<struct pollfd> _fds; // vector of pollfd
		std::string _password;
		std::map<std::string, void (Server::*)(Client&, const Message&)>	_cmds;

	public:
		Server();
		Server(Server const &oth);
		Server &operator=(Server const & oth);
		~Server();

		void	serverInit(int port, const std::string password); // server initialization
		void	serSocket(); // server socket creation
		void	acceptNewClient(); // accept new client
		void	recieveNewData(int fd); // recieve new data from a registered client

		static void signalHandler(int signum); // signal handler

		void	closeFds(); // close file descriptors
		void	clearClients(int fd); // clear clients

		// GETTERS
		int					getSerSocketFd();
		Client* 			getClient(int fd);
		std::string			getPass()	const;

		//---------EXECUTION--------
		void	executeCommand(Client& client, const Message& msg);

		//-----------CMDS----------------
		void	cmdPass(Client& client, const Message& msg);
		void	cmdNick(Client& client, const Message& msg);
		void	cmdUser(Client& client, const Message& msg);
		void	cmdQuit(Client& client, const Message& msg);
		void	cmdJoin(Client& client, const Message& msg);
		void	cmdPart(Client& client, const Message& msg);
		void	cmdPrivmsg(Client& client, const Message& msg);
		void	cmdTopic(Client& client, const Message& msg);
		void	cmdKick(Client& client, const Message& msg);
		void	cmdMode(Client& client, const Message& msg);
		void	cmdInvite(Client& client, const Message& msg);
		void	cmdTry(std::string cmd, Client& client, const Message& msg);
};
