/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dperez-p <dperez-p@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 10:47:46 by dperez-p          #+#    #+#             */
/*   Updated: 2026/08/10 11:03:12 by dperez-p         ###   ########.fr       */
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

class Client
{
private:
	int	_fd; //client file descriptor
	std::string _ipadd; //client ip address
public:
	Client(); // default constr
	int	getFd(); // getter for fd

	void	setFd(int fd); // set fd
	void	setIpAdd(std::string ipadd);
	~Client();
};
