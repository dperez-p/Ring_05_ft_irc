/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dperez-p <dperez-p@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 11:08:52 by dperez-p          #+#    #+#             */
/*   Updated: 2026/09/23 19:29:08 by dperez-p         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server()
{
	this->_serSocketFd = -1;
}

Server::~Server()
{}

Server::Server(Server const &oth)
{
	*this = oth;
}

Server &Server::operator=(Server const &oth)
{
	if (this != &oth)
	{
		this->_port = oth._port;
		this->_serSocketFd = oth._serSocketFd;
		this->_password = oth._password;
		this->_clients = oth._clients;
		this->_channel = oth._channel;
		this->_fds = oth._fds;
	}
	return (*this);
}

/********************************************** Getters *****************************************/
int	Server::getSerSocketFd()
{
	return (_serSocketFd);
}

//Get client Fd from the server vector
Client* Server::getClient(int fd)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i].getFd() == fd)
		{
			return &_clients[i];
		}
	}
	return NULL;
}

// static bool init
bool	Server::_signal = false;

void	Server::signalHandler(int signum)
{
	(void) signum;
	std::cout << std::endl << "Signal received correctly!" << std::endl;
	Server::_signal = true; // set signal to true to stop the server.
}

// close cleints from the server and the server socket.
void	Server::closeFds()
{
	for (size_t i = 0; i < _clients.size(); i++) // close all clients
	{
		std::cout << "Client <" << _clients[i].getFd() << " Disconected." << std::endl;
		close(_clients[i].getFd());
	}
	if (_serSocketFd != -1) // close the server socket.
	{
		std::cout << "Server " << _serSocketFd << "Disconected." << std::endl;
		close(_serSocketFd);
	}
}

//clear clients
void	Server::clearClients(int fd)
{
	for (size_t i = 0; i < _fds.size(); i++) // remove the client from the pollfd
	{
		if (_fds[i].fd == fd)
		{
			_fds.erase(_fds.begin() + i);
			break ;
		}
	}
	for (size_t i = 0; i < _clients.size(); i++) // remove client from the vector of clients
	{
		if (_clients[i].getFd() == fd)
		{
			_clients.erase(_clients.begin() + i);
			break ;
		}
	}
}

// accept new client, kernel checks for the process fd number empty
void	Server::acceptNewClient()
{
	Client	cli; // create client
	struct	sockaddr_in cliadd;
	struct	pollfd newPoll;
	socklen_t	len = sizeof(cliadd);

	int	incfd = accept(_serSocketFd, (sockaddr *)&(cliadd), &len); // accept the new client and register it to the kernel process
	if (incfd == -1)
	{
		std::cout << "accept() failed" << std::endl;
		close(incfd); // release the kernel resource
		return ;
	}
	if (fcntl(incfd, F_SETFL, O_NONBLOCK) == -1) // set the socket option fiir bib-blocking socket
	{
		std::cout << "fcntl() failed" << std::endl;
	}
	newPoll.fd = incfd; // add the client socket to the pollfd
	newPoll.events = POLLIN; // set the event to POLLIN for reading data
	newPoll.revents = 0; // set the revents to 0

	cli.setFd(incfd); // set the client file descriptor
	cli.setIpAdd(inet_ntoa((cliadd.sin_addr))); // convert the ipaddress to string and set
	_clients.push_back(cli); // add client to the vector of clients
	_fds.push_back(newPoll); // add the client socket to the pollfdl

	std::cout << "Client <" << incfd << "> Connected" << std::endl;
}

// New data management
void	Server::recieveNewData(int fd)
{
	char	buff[1024]; // buffer for the data
	memset(buff, 0, sizeof(buff)); // clear the buffer
	Client* actualClient = getClient(fd);
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0); // recive the data

	if (bytes <= 0) // check if the client disconnected
	{
		std::cout << "Client " << fd << "disconnected." << std::endl;
		clearClients(fd); // clear the client
		close(fd);
		return ;
	}
	else // print the recieved data
	{
		actualClient->setBuffer(buff);
		if (actualClient->getBuffer().find_first_of("\r\n") == std::string::npos)
			return ;
		std::vector<std::string> commands = actualClient->splitBuffer();
		buff[bytes] = '\0';
		std::cout << "Client <" << fd << "> data " << buff;
		//here you can add your code to process the received data: parse, check, authenticate, handle the command, etc...
	}
}

// server socket creation.
void	Server::serSocket()
{
	struct	sockaddr_in	add; // default sockeadd structure.
	struct	pollfd	newPoll; //default poll structure.
	add.sin_family = AF_INET; // set the address family to IPV4
	add.sin_port = htons(this->_port); // conver te port to network bye order (big endian)
	add.sin_addr.s_addr = INADDR_ANY; // set the adress to any Local machine address

	_serSocketFd = socket(AF_INET, SOCK_STREAM, 0); // creathe the server socket from the default socket function.
	if (_serSocketFd == -1) // check if fail
	{
		throw(std::runtime_error("faild to create socket."));
	}

	int	num = 1;
	if (setsockopt(_serSocketFd, SOL_SOCKET, SO_REUSEADDR, &num, sizeof(num)) == -1) // set the socket option (SO_REUSEADDR) to reuse the address with default setsockopt function skiping time_wait
	{
		throw(std::runtime_error("faild to set option SO_REUSERADDR on socket."));
	}
	if (fcntl(_serSocketFd, F_SETFL, O_NONBLOCK) == -1) // set the socket option (O_NONBLOCK) for not blocking socket with fcnlt default function
	{
		throw(std::runtime_error("faild to set O_NONBLOCK on socket."));
	}
	if (bind(_serSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1) // bind the socket to the address and port to enable other processes to communicate with it over the network
	{
		throw(std::runtime_error("faild to bind the socket to the adress."));
	}
	if (listen(_serSocketFd, SOMAXCONN) == -1) // listen for incoming connection and making the socket a passive socket
	{
		throw(std::runtime_error("listen() faild."));
	}

	newPoll.fd = _serSocketFd; // add the server socket to the poll.fd
	newPoll.events = POLLIN; // set the event to default POLLIN for reading
	newPoll.revents = 0; //set the revents to 0
	_fds.push_back(newPoll); // add the server socket structure to the pollfds;
}

// server init.
void	Server::serverInit(int port, const std::string password)
{
	_port = port;
	_password = password;
	serSocket(); // create the server socket

	std::cout << "Server: " << _serSocketFd << " connected." << std::endl;
	std::cout << "Waiting to accept a connection... \n";

	while (Server::_signal == false) // run the server until the signal is received
	{
		if ((poll(&_fds[0],_fds.size(), -1) == -1) && Server::_signal == false)
		{
			throw(std::runtime_error("poll() failed"));
		}
		for (size_t i = 0; i < _fds.size(); i++)
		{
			if (_fds[i].revents & POLLIN) // check if there is data to read
			{
				if (_fds[i].fd == _serSocketFd)
				{
					acceptNewClient(); // accept new client
				}
				else
				{
					recieveNewData(_fds[i].fd);
				}
			}
		}
	}
	closeFds(); // close the file descriptors when the server stops
}
