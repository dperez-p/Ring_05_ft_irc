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
		std::vector<Client*>	_clients;
		std::vector<Client*>	_invited;
		std::vector<Client*>	_operators;

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
		void	invite(Client& Client);
		std::string	getTopic() const;
		void	setTopic(const std::string& topic);
		void	setInvite(const bool value);
		void	setTopicLock(const bool value);
		void	setKey(const std::string newkey);
		void	setLimit(int limit);
		void	changeOperatorStatus(Client& client);
};


