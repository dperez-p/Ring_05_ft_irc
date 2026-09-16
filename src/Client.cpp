/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dperez-p <dperez-p@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 11:14:48 by dperez-p          #+#    #+#             */
/*   Updated: 2026/09/16 09:54:24 by dperez-p         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

// Default constructor
Client::Client()
{};

// get client _fd
int	Client::getFd()
{
	return (_fd);
}

// set client _fd
void	Client::setFd(int	fd)
{
	_fd = fd;
}

// set client ip address
void	Client::setIpAdd(std::string ipadd)
{
	_ipadd = ipadd;
}
