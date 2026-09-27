/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Replies.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lanton-m <lanton-m@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/27 19:31:35 by lanton-m          #+#    #+#             */
/*   Updated: 2026/09/27 21:15:34 by lanton-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Message.hpp"
#include "Server.hpp"

std::string   unknownCmd(std::string nick, std::string cmd)
{
    std::string  frase;
    
    if (!nick.empty())
        frase = ":ircserv 421 " + nick + " " + cmd + " :Unknown command";
    else
        frase = ":ircserv 421 * " + cmd + " :Unknown command";
        
    return frase;
}

std::string    notRegUser(std::string nick)
{
    std::string frase;

    if (!nick.empty())
        frase = ":ircserv 451 " + nick + " " + " :You have not registered";
    else
        frase = ":ircserv 451 * :You have not registered";
        
    return frase;
}

std::string insParams(std::string nick, std::string cmd)
{
    return ":ircserv 461 " + nick + " " + cmd + " :Not enough parameters";
}

std::string alrdySign(std::string nick)
{ 
    return ":ircserv 462 " + nick + " :You may not reregister";
}

std::string paswdMiss(std::string nick)
{
    std::string frase;
    
    if (!nick.empty())
        frase = ":ircserv 464 " + nick + " " + " :Password incorrect";
    else
        frase = ":ircserv 464 * :Password incorrect";
        
    return frase;
}

std::string noNick(std::string nick)
{
    return "ircserv 431 * :No nickname given";
}

std::string nickInUse(std::string nick)
{
    return "ircserv 433 * " + nick + " :Nickname is already in use";   
}

std::string noMatch(std::string value)
{
    return "ircserv 401"
}