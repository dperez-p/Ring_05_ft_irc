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
#include "Channel.hpp"

// Default constructor
Client::Client()
{
	this->_nickname = "";
	this->_username = "";
	this->_fd = -1;
	this->_registered = false;
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
		this->_ipadd = oth._ipadd;
	}
	return (*this);
}

// get client _fd
int	Client::getFd() const
{
	return (_fd);
}

std::string	Client::getNick() const
{
	return (_nickname);
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
