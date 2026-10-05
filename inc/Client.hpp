/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dperez-p <dperez-p@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 10:47:46 by dperez-p          #+#    #+#             */
/*   Updated: 2026/10/05 12:49:31 by dperez-p         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include <sys/socket.h> //socket()
#include <sys/types.h> //for socket() too
#include <netinet/in.h> // sockaddr_in()
#include <fcntl.h> // for fcntl()
#include <arpa/inet.h> // for inet_ntoa()
#include <poll.h> // for poll()
#include <csignal> //for signal()
#include "Server.hpp"

class	Channel;
class Client
{
 private:
	std::string	_nickname; // user nickname
	std::string	_username; // username
	bool		_isOperator; // is operator (mod for the channel)
	bool		_registered; // is registered
	bool		_logged; // is logged
	bool		_overSized; // control if the buffer was oversized
	int			_fd;	//client file descriptor
	std::string _ipadd; //client ip address
	std::string _recvBuffer; // client buffer

	public:
		Client(); // default constr
		Client(std::string nickname, std::string username, int fd); // argu constr
		Client(Client const &oth); // copy construct
		Client &operator=(Client const &other); // assignment operator
		~Client(); // desctruct


	void							setFd(int fd); // set fd
	void							setIpAdd(std::string ipadd);

  	/************************Getter*************************** */
	int					getFd() const; // getter for fd
	int					getBufferSize() const;
	const std::string&			getBuffer() const;
	std::string						getNick() const;
	std::string						getPrefix() const;
	bool							getIsLogged() const;
	bool							getIsRegistered() const;
	bool							getIsOverSized() const;

	/***************************Setter******************************* */
	void								appendBuffer(const char* data, ssize_t len);
	void							setBuffer(std::string bytes);
	void							setLogged(bool state);
	void							setNick(const std::string& nickname);
	void							setUser(const std::string& username);
	void							setRegistered();

	/***************************Parsing****************************** */
	std::vector<std::string>		splitBuffer();
	void							clearBuffer();

	bool		isOperator(const Channel& channel) const;
	bool		inChannel(const Channel& channel) const;
	bool		isInvited(const Channel& channel) const;
	bool		isRegistered() const;
	std::string	nickForReplay() const;
};


