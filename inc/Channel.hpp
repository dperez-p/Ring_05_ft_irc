/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ramarti2 <ramarti2@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 22:59:59 by lanton-m          #+#    #+#             */
/*   Updated: 2026/10/08 14:49:01 by ramarti2         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Client.hpp"
#include "Replies.hpp"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

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
		void	addClient(Client& client, const std::string& key);
		void	part(Client& client, const std::string& comment);
		void	kick(Client& kicker, Client& toKick, const std::string& comment);
		void	invite(Client& inviter, Client& toInvite);
		void	topic(Client& caller, const std::string& newTopic, bool onlyView);
		void	showMode(Client& caller);
		void	setMode(Client& caller, const std::string& modestr, const std::vector<std::string>& args);
	// Setters:
		void	setTopic(const std::string& topic);
		void	setInviteOnly(const bool value);
		void	setTopicLock(const bool value);
		void	setKey(const std::string newkey);
		void	setLimit(int limit);
		void	setOperatorStatus(Client& setter, const std::string& nickname, bool setting);

	// Getters:
		const std::string&			getTopic() const;
		const std::vector<Client*>&	getOperators() const;
		const std::vector<Client*>&	getClients() const;
		const std::vector<Client*>&	getInvites() const;
		const std::string& 			getName() const;
		bool	isInviteOnly() const;


		void	broadcast(const std::string& message, const Client* exclude = NULL);

		void	removeClient(Client& client);

};

void	send_msg(Client& recvr, const std::string toSend);		//Añado send_msg al hpp para poder llamarla desde fuera

