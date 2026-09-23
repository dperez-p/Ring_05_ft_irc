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

void	Channel::kick(const std::string& uname, const std::string& comment)
{
	// Assuming usernames are unique
	std::vector<Client>::iterator it = _clients.begin();
	for (; it < _clients.end(); it++)
	{
		if (it->getUsername() == uname)
		{
			std::cout << it->getUsername() << " was kicked from " << _name;
			if (!comment.empty())
				std::cout << " because " << comment;
			std::cout << std::endl;
			_clients.erase(it);
			return ;
		}
	}
	std::cout << "No user called " << uname << " in " << _name << " ." << std::endl;
}

void	Channel::invite(const Client& Client)
{
		
}

std::string	Channel::getTopic()
{

}

void	Channel::setTopic(const std::string& topic)
{

}

void	Channel::setMode(char mode)
{

}