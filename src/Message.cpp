/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lanton-m <lanton-m@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/27 13:27:14 by lanton-m          #+#    #+#             */
/*   Updated: 2026/09/29 22:48:51 by lanton-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"
#include <cctype>

Message::Message(std::string line)
{
    std::vector<std::string>        args;
    std::size_t                     pos;
    std::string                     tmp;
    int                             i = 0;

    pos = line.find(' ');
    _cmd = line.substr(0, pos);
    while (i < _cmd.size())
        _cmd[i] = std::toupper(static_cast<unsigned char>(_cmd[i++]));
    while (pos != std::string::npos)
    {
        line.erase(0, pos + 1);
        while (!line.empty() && line[0] == ' ') line.erase(0, 1);
        if (!line.empty() && line[0] == ':')
        {
            _params.push_back(line.substr(1, line.size()));
            break;
        }
        pos = line.find(' ');
        _params.push_back(line.substr(0, pos));
    }
}
Message::Message(const Message& other)
{
    *this = other;
}

Message &Message::operator=(const Message& other)
{
    if (this != &other)
    {
        _cmd = other._cmd;
        _params = other._params;
    }
    return *this;
}

Message::~Message(){}


std::string Message::getCmd() const
{
    return _cmd;
}

std::vector<std::string> Message::getParam() const
{
    return _params;
}
