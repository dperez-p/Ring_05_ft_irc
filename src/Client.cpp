/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dperez-p <dperez-p@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 11:14:48 by dperez-p          #+#    #+#             */
/*   Updated: 2026/09/23 20:06:28 by dperez-p         ###   ########.fr       */
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
int Client::getFd() const
{
	return (_fd);
}

//Return the client buffer
const std::string&  Client::getBuffer() const
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

// split the buffer and erase it
std::vector<std::string> Client::splitBuffer()
{
	std::vector<std::string> lines;
	size_t pos;
	//Find the delimiter and extract the line, then erase it
	while ((pos = _recvBuffer.find_first_of("\r\n")) != std::string::npos)
	{
		//EDGE CASE if we found '\r' at the very end of the buffer, wait for potential '\n' in next recv
		if (_recvBuffer[pos] == '\r' && pos + 1 == _recvBuffer.size())
		{
			break; // Stop parsing for now, wait for more data
		}
		std::string	line = _recvBuffer.substr(0, pos);
		if (!line.empty())
		{
			lines.push_back(line);
		}
		// check the size then check bot \r && \n to erase it
		if (pos + 1 < _recvBuffer.size() && _recvBuffer[pos] == '\r' &&  _recvBuffer[pos + 1] == '\n' )
		{
			_recvBuffer.erase(0, pos + 2);
		}
		// erase until \n
		else
		{
			_recvBuffer.erase(0, pos + 1);
		}
	}
	return lines;
}
