/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ramarti2 <ramarti2@student.42malaga.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 22:59:59 by lanton-m          #+#    #+#             */
/*   Updated: 2026/09/30 17:09:24 by ramarti2         ###   ########.fr       */
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
	// Note: We assume that the client executing these is an operator. No checks.
		void	addClient(Client& client, const std::string& key);
		void	kick(Client& kicker, Client& toKick, const std::string& comment);
		void	invite(Client& Client); // 
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

		
		/*
		LUIS TODO: 
		1. hacer funcion de JOIN que al final es un wrapper para esta.
			- Comprueba si el canal existe, si no, lo crea y llamas mi addClient que convierte al cliente en operador por defecto.
			- Si el cliente mete un key como parametro, pasaselo a mi función.
		2. 
		
		*/


		//TODO:
		void	showMode(Client& caller);
		// set several modes at once?
		//void	Channel::setModes(const std::string& modes)
		// modify kick so no messages are printed when leave voluntarily
		// create "broadcast" func that sends to channel only
		void	broadcast(const std::string& message, const Client* exclude = NULL);
		// figure out how to demand input from clients for passkeys
		//
};


