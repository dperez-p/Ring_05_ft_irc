/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Replies.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lanton-m <lanton-m@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/27 21:20:38 by lanton-m          #+#    #+#             */
/*   Updated: 2026/09/27 21:22:16 by lanton-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include "Server.hpp"
#include "Message.hpp"

std::string  unknownCmd(std::string nick, std::string cmd);
std::string  notRegUser(std::string nick);
std::string  insParams(std::string nick, std::string cmd);
std::string  alrdySign(std::string nick);
std::string  paswdMiss(std::string nick);
std::string  noNick(std::string nick);
std::string  nickInUse(std::string nick);
std::string  noMatch(std::string value);


