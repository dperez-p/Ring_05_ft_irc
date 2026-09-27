/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lanton-m <lanton-m@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/27 13:27:02 by lanton-m          #+#    #+#             */
/*   Updated: 2026/09/27 15:51:02 by lanton-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include "Server.hpp"

class Message
{
    private:
        std::string                 _cmd;
        std::vector<std::string>    _params;
        
    public:
        Message(std::string);
        Message(const Message& other);
        Message &operator=(const Message& other);
        ~Message();

        //----------------getters----------------------
        std::string                 getCmd()    const;
        std::vector<std::string>    getParam()  const;
        
};