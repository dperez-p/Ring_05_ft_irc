/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lanton-m <lanton-m@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 23:00:52 by lanton-m          #+#    #+#             */
/*   Updated: 2026/09/30 19:33:24 by lanton-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../inc/Channel.hpp"

void	send_msg(Client& recvr, const std::string toSend)
{
	send(recvr.getFd(), toSend.c_str(), toSend.length(), MSG_NOSIGNAL);
}

// ------------------- OCF + Parameterized Constructor --------------
Channel::Channel()
{
	_protectedTopic = _inviteOnly = false;
	_userLimit = 0; // 0 = unlimited
	_topic = "";
	_name = "channel";
	_key = "";
}

Channel &Channel::operator=(const Channel& other)
{
	if (this != &other)
	{
		_protectedTopic = other._protectedTopic;
		_inviteOnly = other._inviteOnly;
		_userLimit = other._userLimit;
		_name = other._name;
		_topic = other._topic;
		_key = other._key;
		_clients = other._clients;
		_invited = other._invited;
		_operators = other._operators;
	}
	return (*this);
}

Channel::Channel(const Channel& other)
{
	*this = other;
}

Channel::Channel(const std::string& name, const std::string& key) : Channel()
{
	_name = name;
	_key = key;
}

Channel::~Channel() {}

// ---------- Channel Operations -------------
// channel operator verification occurs OUTSIDE these

int	search(const std::string& nickname, std::vector<Client*>& list)
{
	for (int i = 0; i < static_cast<int>(list.size()); i++)
	{
		if (list[i]->getNick() == nickname)
			return i;
	}
	return -1;
}

void	Channel::kick(Client& kicker, Client& toKick, const std::string& comment)
{
	int i;

	if (search(kicker.getNick(), _clients) == -1)
		send_msg(kicker, ERR_NOTONCHANNEL(kicker.getNick(), _name));

	if (search(kicker.getNick(), _operators) == -1)
		send_msg(kicker, ERR_CHANOPRIVSNEEDED(kicker.getNick(), _name));

	i = search(toKick.getNick(), _clients);
	if (i == -1)
		send_msg(toKick, ERR_USERNOTINCHANNEL(toKick.getNick(), _name));
	_clients.erase(_clients.begin() + i);

	i = search(toKick.getNick(), _invited);
	if (i > -1)
		_invited.erase(_invited.begin() + i);

	i = search(toKick.getNick(), _operators);
	if (i > -1)
		_operators.erase(_operators.begin() + i);

	std::string joinMsg =
	":" + kicker.getPrefix() + " KICK #" + _name + " " + toKick.getNick() + " :" + comment + "\r\n";
	broadcast(joinMsg);
}


void	Channel::invite(Client& client)
{
	// Do I print a message for invites? Where?
	for (int i = 0; i < (int)_invited.size(); i++)
	{
		if (client.getNick() == _invited[i]->getNick())
		{
			std::cout << "User was already invited." << std::endl;
			return;
		}
	}
	_invited.push_back(&client);
}

//void	Channel::add()
// TOPIC command
const std::string&	Channel::getTopic() const
{
	return _topic;
}

void	Channel::setTopic(const std::string& topic)
{
	_topic = topic;
}
// mode 'i'
void	Channel::setInvite(const bool value)
{
	_inviteOnly = value;
}
// mode 't'
void	Channel::setTopicLock(const bool value)
{
	_protectedTopic = value;
}
// mode 'k'
void	Channel::setKey(const std::string newkey)
{
	_key = newkey;
}
// mode 'l'
void	Channel::setLimit(int limit)
{
	_userLimit = limit;
}

// mode 'o'
void	Channel::setOperatorStatus(Client& client, bool setting)
{
	if (!client.inChannel(*this))
	{
		std::cout << "Client not in channel." << std::endl;
		return ;
	}

	// check if client in operators
	bool inList = false;
	int i = 0;
	for (; i < (int)_operators.size() && !inList; i++)
	{
		if (_operators[i]->getNick() == client.getNick())
			inList = true;
	}
	// erase or add to _operators
	if (inList)
		_operators.erase(_operators.begin() + i);
	if(setting == true)
		_operators.push_back(&client);
}

const std::vector<Client*>&	Channel::getOperators() const
{
	return _operators;
}

const std::vector<Client*>&	Channel::getClients() const
{
	return _clients;
}

const std::vector<Client*>&	Channel::getInvites() const
{
	return _invited;
}

bool	Channel::isInviteOnly() const
{
	return _inviteOnly;
}



void	Channel::showMode(Client& caller)
{
	std::string modes("+");
	std::string values;
	//itkl
	if (isInviteOnly())
		modes += "i";
	if (_protectedTopic)
		modes += "t";
	if (_key != "")
	{
		modes += "k";
		values += _key;
	}
	if (_userLimit > -1)
	{
		modes += "l";
		if (_key != "")
			values += " ";
		values += std::to_string(_userLimit);
	}
	send_msg(caller, RPL_CHANNELMODEIS(caller.getNick(), _name, modes, values));
}

void	Channel::setMode(Client& caller)
{


}


void Channel::broadcast(const std::string& message, const Client* exclude)
{
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		// Skip the client if they match the exclude pointer
		if (exclude != NULL && _clients[i] == exclude)
			continue;
		send(_clients[i]->getFd(), message.c_str(), message.length(), MSG_NOSIGNAL);
	}
}

void	Channel::addClient(Client& client, const std::string& key)
{
	std::string err;
	if (_key != "" && key != _key)
	{
		send_msg(client, ERR_BADCHANNELKEY(client.getNick(), _name));
		return ;
	}

	if (_userLimit && static_cast<int>(_clients.size()) >= _userLimit)
	{
		send_msg(client, ERR_CHANNELISFULL(client.getNick(), _name));
		return ;
	}

	if (_inviteOnly && !client.isInvited(*this))
	{
		send_msg(client, ERR_INVITEONLYCHAN(client.getNick(), _name));
		return ;
	}

	// Remove from _invited
	for (int i = 0; i < static_cast<int>(_invited.size()); i++)
	{
		if (client.getNick() == _invited[i]->getNick())
		{
			_invited.erase(_invited.begin() + i);
			break ;
		}
	}

	if (_clients.size() == 0)
	{
		_operators.push_back(&client);
	}
	_clients.push_back(&client);

	// Broadcast join message (TODO: replace)
	std::string joinMsg = ":" + client.getPrefix() + " JOIN :" + _name + "\r\n";
	broadcast(joinMsg);

	// Send Topic Reply (RPL_TOPIC 332 or RPL_NOTOPIC 331)
	if (!_topic.empty())
	{
		send_msg(client, RPL_TOPICIS(client.getNick(), _name, _topic));
	}
	else
	{
		send_msg(client, RPL_NOTOPIC(client.getNick(), _name));
	}

	// Member List Sequence (RPL_NAMREPLY 353 & RPL_ENDOFNAMES 366)
	std::string names;
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (i > 0)
			names += " ";
		if (_clients[i]->isOperator(*this))
			names += "@"; // '@' prefix for operators
		names += _clients[i]->getNick();
	}

	send_msg(client, RPL_NAMREPLY(client.getNick(), _name, names));
	send_msg(client, RPL_ENDOFNAMES(client.getNick(), _name));
}
