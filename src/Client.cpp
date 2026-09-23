/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dperez-p <dperez-p@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 11:14:48 by dperez-p          #+#    #+#             */
/*   Updated: 2026/09/20 13:16:00 by dperez-p         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

// Default constructor
Client::Client()
{
	this->_nickname = "";
	this->_username = "";
	this->_fd = -1;
	this->_isOperator = false;
	this->_registered = false;
	this->_recvBuffer = "";
	this->_ipadd = "";
	this->_loged = false;
};

Client::Client(std::string nickname, std::string username, int fd)
{
	this->_nickname = nickname;
	this->_username = username;
	this->_fd = fd;
	this->_isOperator = false;
	this->_registered = false;
	this->_recvBuffer = "";
	this->_ipadd = "";
	this->_loged = false;
}

Client::Client(Client const &oth)
{
	*this = oth;
}

Client& Client::operator=(Client const &oth)
{
	if (this != &oth)
	{
		this->_nickname = oth._nickname;
		this->_username = oth._username;
		this->_fd = oth._fd;
		this->_isOperator = oth._isOperator;
		this->_registered = oth._registered;
		this->_recvBuffer = oth._recvBuffer;
		this->_loged = oth._loged;
		this->_ipadd = oth._ipadd;
	}
	return (*this);
}
/*********************************Getters*************************************** */
// get client _fd
int	const Client::getFd() const
{
	return (_fd);
}

//Return the client buffer
std::string  Client::getBuffer() const
{
	return (_recvBuffer);
}



/**********************************Setters***********************8 */
// set client _fd
void	Client::setFd(int	fd)
{
	_fd = fd;
}

// set client ip address
void	Client::setIpAdd(std::string ipadd)
{
	_ipadd = ipadd;
}

// add to the current client buffer.
void	Client::setBuffer(std::string buff)
{
	_recvBuffer += buff;
}

std::vector<string> Client::splitBuffer()
{
	std::vector<std::string> line; 
	

	return ()
}