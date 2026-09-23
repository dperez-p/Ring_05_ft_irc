#pragma once
#include "Client.hpp"

class Channel
{
	private:
		bool _protectedTopic;
		bool _inviteOnly;
		int _userLimit;
		std::string	_name;
		std::string	_topic;
		std::string	_key ;
		std::vector<Client *>	_clients;
		// TODO: initialize _clients in constructors!!

	public:
	// OCF:
		Channel();
		Channel &operator=(const Channel& other);
		Channel(const Channel& other);
		Channel(const std::string& name, const std::string& key);
		// ^^ Since JOIN only has 'channel' and 'key' parameters
		~Channel();
	
	// For channel operators:
		void	kick(const std::string& uname, const std::string& comment);
		void	invite(const Client& Client);
		std::string	getTopic();
		void	setTopic(const std::string& topic);
		void	setMode(char mode);
};


