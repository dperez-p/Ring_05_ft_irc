/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lanton-m <lanton-m@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 11:14:48 by dperez-p          #+#    #+#             */
/*   Updated: 2026/09/24 13:32:17 by dperez-p         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Client.hpp"

// Default constructor
Client::Client()
{
	this->_nickname = "";
	this->_username = "";
	this->_fd = -1;
	this->_registered = false;
	this->_overSized = false;
	this->_recvBuffer = "";
	this->_ipadd = "";
	this->_logged = false;
};

Client::Client(std::string nickname, std::string username, int fd)
{
	this->_nickname = nickname;
	this->_username = username;
	this->_fd = fd;
	this->_registered = false;
	this->_overSized = false;
	this->_recvBuffer = "";
	this->_ipadd = "";
	this->_logged = false;
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
		this->_registered = oth._registered;
		this->_recvBuffer = oth._recvBuffer;
		this->_logged = oth._logged;
		this->_overSized = oth._overSized;
		this->_ipadd = oth._ipadd;
	}
	return (*this);
}

Client::~Client()
{

}

/*********************************Getters*************************************** */
// get client _fd
int Client::getFd() const
{
	return (_fd);
}

bool	Client::getIsLogged() const
{
	return _logged;
}

bool	Client::getIsRegistered() const
{
	return _registered;
}

std::string	Client::getNick() const
{
	return (_nickname);
}

std::string	Client::getPrefix() const
{
	return (_nickname + "!" + _username + "@" + _ipadd);
}

//Return the client buffer
const std::string&  Client::getBuffer() const
{
	return (_recvBuffer);
}

int Client::getBufferSize() const
{
	return _recvBuffer.size();
}

bool	Client::getIsOverSized() const
{
	return _overSized;
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
void Client::appendBuffer(const char* data, ssize_t len)
{
	_recvBuffer.append(data, len);
}

void	Client::setLogged(bool state)
{
	_logged = state;
}

void	Client::setNick(const std::string& nickname)
{
	_nickname = nickname;
}

// split the buffer and erase it
std::vector<std::string> Client::splitBuffer()
{
	std::vector<std::string> lines;
	size_t pos;
	static const	size_t	maxMessageSize = 512;
	//Find the delimiter and extract the line, then erase it
	while ((pos = _recvBuffer.find_first_of("\r\n")) != std::string::npos)
	{
		//EDGE CASE if we found '\r' at the very end of the buffer, wait for potential '\n' in next recv
		if (_recvBuffer[pos] == '\r' && pos + 1 == _recvBuffer.size())
		{
			if (_recvBuffer.size() > maxMessageSize)
			{
				_recvBuffer.clear();
				_overSized = true;
				return lines;
			}
			break; // Stop parsing for now, wait for more data
		}
		std::string	line = _recvBuffer.substr(0, pos);
		if (line.size() > maxMessageSize) //Truncate, is a valid message,just too long
		{
			line = line.substr(0, maxMessageSize - 2); // keep only the first 512 bytes, discard the rest
		}
		if (!line.empty())
		{
			lines.push_back(line);
		}
		// check the size then check both \r && \n to erase it
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
	if (_recvBuffer.size() > maxMessageSize)
	{
		_recvBuffer.clear();
		_overSized = true;
		return lines;
	}
	return lines;
}

void	Client::clearBuffer()
{
	_recvBuffer.clear();
}

bool	Client::isOperator(const Channel& channel) const
{
	std::vector<Client*> opts = channel.getOperators();

	for (int i = 0; i < (int)opts.size(); i++)
	{
		if (opts[i]->getNick() == this->getNick())
			return true;
	}
	return false;
}

bool	Client::inChannel(const Channel& channel) const
{
	std::vector<Client*> clients = channel.getClients();

	for (int i = 0; i < (int)clients.size(); i++)
	{
		if (clients[i]->getNick() == this->getNick())
			return true;
	}
	return false;
}

bool	Client::isInvited(const Channel& channel) const
{
	std::vector<Client*> invites = channel.getInvites();
	for (int i = 0; i < (int)invites.size(); i++)
	{
		if (invites[i]->getNick() == this->getNick())
			return true;
	}
	return false;
}


bool	Client::isRegistered() const
{
	return (!_nickname.empty() && !_username.empty() && _logged);
}

std::string	Client::nickForReplay() const
{
	if (_nickname.empty()) return ("*");
	return (_nickname);
}
