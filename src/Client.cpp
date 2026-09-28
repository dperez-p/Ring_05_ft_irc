/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lanton-m <lanton-m@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 11:14:48 by dperez-p          #+#    #+#             */
/*   Updated: 2026/09/27 21:42:55 by lanton-m         ###   ########.fr       */
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
}

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

// get client _fd
int const	Client::getFd()
{
	return (_fd);
}

std::string	Client::getNick()
{
	return _nickname;
}

bool	Client::getLoged()
{
	return _loged;
}

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

void	Client::setLoged(bool state)
{
	_loged = state;
}

std::vector<std::string> Client::splitBuffer()
{
	std::vector<std::string> lines;
	std::size_t pos;

	pos = _recvBuffer.find('\n');
	while (pos != std::string::npos)
	{
		std::string	line = _recvBuffer.substr(0, pos);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		lines.push_back(line);
		_recvBuffer.erase(0, pos + 1);
		pos = _recvBuffer.find('\n');
	}
	return (lines);
}

void	Client::clearBuffer()
{
	_recvBuffer.clear();
}
