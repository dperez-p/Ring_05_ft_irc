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
		void	setTopic(const std::string& topic);
		void	setInvite(const bool value);
		void	setTopicLock(const bool value);
		void	setKey(const std::string newkey);
		void	setLimit(int limit);
		void	setOperatorStatus(Client& client, bool isoperator);
		
		const std::string&	getTopic() const;
		const std::vector<Client*>&	getOperators() const;
		const std::vector<Client*>&	getClients() const;
		const std::vector<Client*>&	getInvites() const;
		bool	isInviteOnly() const;

		void	addClient(Client& client);

		//TODO:
		// show mode.
		// set several modes at once?
		// modify kick so no messages are printed when leave voluntarily
		// create "broadcast" func that sends to channel only
		// figure out how to demand input from clients for passkeys
		// 
};


