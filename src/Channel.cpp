/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ramarti2 <ramarti2@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 23:00:52 by lanton-m          #+#    #+#             */
/*   Updated: 2026/10/08 14:48:26 by ramarti2         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../inc/Channel.hpp"
#include "../inc/Client.hpp"
#include <cstdlib>
#include <sstream>

// ======================================= Helpers =====================================================
void	send_msg(Client& recvr, const std::string toSend)
{
	send(recvr.getFd(), toSend.c_str(), toSend.length(), MSG_NOSIGNAL);
}

int	search(const std::string& nickname, std::vector<Client*>& list)
{
	for (int i = 0; i < static_cast<int>(list.size()); i++)
	{
		if (list[i]->getNick() == nickname)
			return i;
	}
	return -1;
}

// ============================= OCF + Parameterized Constructor ==================================================
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

Channel::Channel(const std::string& name, const std::string& key)
: _protectedTopic(false), _inviteOnly(false), _userLimit(0), _name(name), _topic(""), _key(key) {}

Channel::~Channel() {}

// ======================================= Getters =====================================================
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

const std::string&	Channel::getTopic() const
{
	return _topic;
}

const std::string& Channel::getName() const
{
	return _name;
}

bool	Channel::isInviteOnly() const
{
	return _inviteOnly;
}

// ======================================= Channel Operations =====================================================
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
	int i = search(client.getNick(), _invited);
	if (i != -1)
		_invited.erase(_invited.begin() + i);

	if (_clients.size() == 0)
		_operators.push_back(&client);
	_clients.push_back(&client);
// broadcast JOIN message
	std::string joinMsg = ":" + client.getPrefix() + " JOIN #" + _name + "\r\n";
	broadcast(joinMsg);
// Send Topic Reply (RPL_TOPIC 332 or RPL_NOTOPIC 331)
	if (!_topic.empty())
		send_msg(client, RPL_TOPICIS(client.getNick(), _name, _topic));
	else
		send_msg(client, RPL_NOTOPIC(client.getNick(), _name));
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

void	Channel::part(Client& client, const std::string& comment)
{
	int i = search(client.getNick(), _clients);
	if (i == -1)
		return send_msg(client, ERR_NOTONCHANNEL(client.getNick(), _name));

	_clients.erase(_clients.begin() + i);

	i = search(client.getNick(), _operators);
	if (i != -1)
		_operators.erase(_operators.begin() + i);

	i = search(client.getNick(), _invited);
	if (i != -1)
		_invited.erase(_invited.begin() + i);

	std::string partMsg =
	":" + client.getPrefix() + " PART #" + _name + " :" + comment + CRLF;
	broadcast(partMsg);
}

void	Channel::kick(Client& kicker, Client& toKick, const std::string& comment)
{
	int i;

	if (search(kicker.getNick(), _clients) == -1)
		return send_msg(kicker, ERR_NOTONCHANNEL(kicker.getNick(), _name));

	if (search(kicker.getNick(), _operators) == -1)
		return send_msg(kicker, ERR_CHANOPRIVSNEEDED(kicker.getNick(), _name));

	i = search(toKick.getNick(), _clients);
	if (i == -1)
		return send_msg(toKick, ERR_USERNOTINCHANNEL(kicker.getNick(), toKick.getNick(), _name));
	_clients.erase(_clients.begin() + i);

	i = search(toKick.getNick(), _invited);
	if (i > -1)
		_invited.erase(_invited.begin() + i);

	i = search(toKick.getNick(), _operators);
	if (i > -1)
		_operators.erase(_operators.begin() + i);

	std::string kickMsg =
	":" + kicker.getPrefix() + " KICK #" + _name + " " + toKick.getNick() + " :" + comment + CRLF;
	broadcast(kickMsg);
}

void	Channel::invite(Client& inviter, Client& toInvite)
{
	if (search(inviter.getNick(), _clients) == -1)
		return send_msg(inviter, ERR_NOTONCHANNEL(inviter.getNick(), _name));

	if (_inviteOnly && search(inviter.getNick(), _operators) == -1)
		return send_msg(inviter, ERR_CHANOPRIVSNEEDED(inviter.getNick(), _name));

	if (search(toInvite.getNick(), _clients) != -1)
		return send_msg(inviter, ERR_USERONCHANNEL(inviter.getNick(), toInvite.getNick(), _name));

	if (search(toInvite.getNick(), _invited) == -1)
		_invited.push_back(&toInvite);
// Reply to inviter
	send_msg(inviter, RPL_INVITING(inviter.getNick(), toInvite.getNick(), _name));

// Message to invitee
	std::string inviteMsg = ":" + inviter.getPrefix() + " INVITE " + toInvite.getNick() + " #" + _name + CRLF;
	send_msg(toInvite, inviteMsg);
}

// onlyView is there to differentiate between "TOPIC #channel :" (clear topic) and "TOPIC #channel" (view topic).
void	Channel::topic(Client& caller, const std::string& newTopic, bool onlyView)
{
	if (search(caller.getNick(), _clients) == -1)
		return send_msg(caller, ERR_NOTONCHANNEL(caller.getNick(), _name));

	if (_protectedTopic && search(caller.getNick(), _operators) == -1)
		return send_msg(caller, ERR_CHANOPRIVSNEEDED(caller.getNick(), _name));

	if (!onlyView)
	{
		_topic = newTopic;
		// topic change message
		std::string topicChangeMsg =
		caller.getPrefix() + " TOPIC #" + _name + " :" + newTopic + CRLF;
		return broadcast(topicChangeMsg);
	}
	if (!_topic.empty())
		send_msg(caller, RPL_TOPICIS(caller.getNick(), _name, _topic));
	else
		send_msg(caller, RPL_NOTOPIC(caller.getNick(), _name));
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
	if (_userLimit > 0)
	{
		std::ostringstream limit;

		modes += "l";
		if (_key != "")
			values += " ";
		limit << _userLimit;
		values += limit.str();
	}
	send_msg(caller, RPL_CHANNELMODEIS(caller.getNick(), _name, modes, values));
}

void	Channel::setMode(Client& caller, const std::string& modestr, const std::vector<std::string>& args)
{
	if (search(caller.getNick(), _operators) == -1)
		return send_msg(caller, ERR_CHANOPRIVSNEEDED(caller.getNick(), _name));

	std::vector<std::string>::const_iterator it = args.begin();

	bool	addMode = true;
	for (size_t i = 0; i < modestr.length(); i++)
	{
		if (modestr.at(i) == '+')
		{
			addMode = true;
		}
		else if (modestr.at(i) == '-')
		{
			addMode = false;
		}
		else if (modestr.at(i) == 'i')
		{
			this->setInviteOnly(addMode);
		}
		else if (modestr.at(i) == 't')
		{
			this->setTopicLock(addMode);
		}
		else if (modestr.at(i) == 'k')
		{
			if (it == args.end())
				return send_msg(caller, ERR_NOTENOUGHPARAM(caller.getNick()));
			if (addMode)
				this->setKey(*it);
			else if (_key == *it)
				this->setKey("");
			it++;
		}
		else if (modestr.at(i) == 'o')
		{
			if (it == args.end())
				return send_msg(caller, ERR_NOTENOUGHPARAM(caller.getNick()));
			this->setOperatorStatus(caller, *it++, addMode);
		}
		else if (modestr.at(i) == 'l')
		{
			if (addMode && it == args.end())
				return send_msg(caller, ERR_NOTENOUGHPARAM(caller.getNick()));
			int value = (addMode == true ? std::atoi((*it++).c_str()) : 0);
			this->setLimit(value);
		}
		else
			return send_msg(caller, ERR_UNKNOWNMODE(caller.getNick(), _name, modestr.at(i)));
	}
}

// ======================================= Setters =====================================================
void	Channel::setTopic(const std::string& topic) { _topic = topic;}
// mode 'i'
void	Channel::setInviteOnly(const bool value) { _inviteOnly = value;}
// mode 't'
void	Channel::setTopicLock(const bool value) { _protectedTopic = value;}
// mode 'k'
void	Channel::setKey(const std::string newkey) {_key = newkey;}
// mode 'l'
void	Channel::setLimit(int limit)
{
	if (limit >= 0)
		_userLimit = limit;
}
// mode 'o'
void	Channel::setOperatorStatus(Client& setter, const std::string& nickname, bool setting)
{
	int i = search(nickname, _clients);
	if (i == -1)
		return send_msg(setter, ERR_USERNOTINCHANNEL(setter.getNick(), nickname, _name));

	if (search(setter.getNick(), _operators) == -1) // might be redundant bc I check in setMode
		return send_msg(setter, ERR_CHANOPRIVSNEEDED(setter.getNick(), _name));

	if (setting == true && search(nickname, _operators) == -1)
		return _operators.push_back(_clients[i]);
	else if (setting == false)
	{
		i = search(nickname, _operators);
		if (i != -1)
			_operators.erase(_operators.begin() + i);
	}
}

//============================ Other =======================================================

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

void	Channel::removeClient(Client& client)
{
	int	i = search(client.getNick(), _clients);
	if (i == -1) // not in this channel
		return ;
	_clients.erase(_clients.begin() + i);

	i = search(client.getNick(), _invited);
	if (i > -1)
	{
		_invited.erase(_invited.begin() + i);
	}

	i = search(client.getNick(), _operators);
	if (i > -1)
		_operators.erase(_operators.begin() + i);

	std::string quitMsg = ":" + client.getPrefix() + " QUIT :Connection closed" + CRLF;
	broadcast(quitMsg);
}
