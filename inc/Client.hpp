/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dperez-p <dperez-p@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 10:47:46 by dperez-p          #+#    #+#             */
/*   Updated: 2026/09/23 19:35:51 by dperez-p         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include <sys/socket.h> //socket()
#include <sys/types.h> //for socket() too
#include <netinet/in.h> // sockaddr_in()
#include <fcntl.h> // for fcntl()
#include <arpa/inet.h> // for inet_ntoa()
#include <poll.h> // for poll()
#include <csignal> //for signal()
#include "Server.hpp"

class Client
{
private:
	std::string	_nickname; // user nickname
	std::string	_username; // username
	bool		_isOperator; // is operator (mod for the channel)
	bool		_registered; // is registered
	bool		_loged; // is loged
	int			_fd;	//client file descriptor
	std::string _ipadd; //client ip address
	std::string _recvBuffer; // client buffer

public:
	Client(); // default constr
	Client(std::string nickname, std::string username, int fd); // argu constr
	Client(Client const &oth); // copy construct
	Client &operator=(Client const &other); // assignment operator
	~Client(); // desctruct

	void	setFd(int fd); // set fd
	void	setIpAdd(std::string ipadd);

	/************************Getter*************************** */
	int	getFd() const; // getter for fd
	const std::string& getBuffer() const;
	/***************************Setter******************************* */
	void	setBuffer(std::string bytes);
	/*******************PARSE*************************** */
	std::vector<std::string>	splitBuffer();
};
