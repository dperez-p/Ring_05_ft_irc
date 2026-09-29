#include "../inc/Channel.hpp"

// ------------------- OCF + Parameterized Constructor --------------
Channel::Channel()
{
	_protectedTopic = _inviteOnly = false;
	_userLimit = -1; // -1 = unlimited
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

void	Channel::kick(const std::string& nickname, const std::string& comment)
{
	for (int i = 0; i < (int)_clients.size(); i++)
	{
		if (_clients[i]->getNick() == nickname)
		{
			std::cout << _clients[i]->getNick() << " was kicked from " << _name;
			if (!comment.empty())
				std::cout << " because " << comment;
			std::cout << std::endl;
			_clients.erase(_clients.begin() + i);
			// TODO remove from _invited and _operators
			return ;
		}
	}
	// Where do I print these messages to?????????  Do I print them at all??
	std::cout << "No user called " << nickname << " in " << _name << " ." << std::endl;
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

/*
	ERR_NEEDMOREPARAMS                                         
	ERR_NOSUCHCHANNEL               ERR_TOOMANYCHANNELS
	RPL_TOPIC
*/

void	Channel::addClient(Client& client, const std::string& key)
{
	std::string err;
	if (_key != "" && key != _key)
	{
		err = ERR_BADCHANNELKEY(_name);
		send(client.getFd(), err.c_str(), err.length(), 0);
		return ;
	}

	if (_userLimit != -1 && _clients.size() >= _userLimit)
	{
		err = ERR_CHANNELISFULL(client.getNick(), _name);
		send(client.getFd(), err.c_str(), err.length(), 0);
		return ;
	}

	if (_inviteOnly && !client.isInvited(*this))
	{
		err = ERR_INVITEONLYCHAN(client.getNick(), _name);
		send(client.getFd(), err.c_str(), err.length(), 0);
		return ;
	}

	// Remove from _invited
	for (int i = 0; i < (int)_invited.size(); i++)
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
	// broadcast join
	// send topic
	// member list sequence RPL_NAMREPLY & RPL_ENDOFNAMES
	err = 
	send(client.getFd(), err.c_str(), err.length(), 0);
}