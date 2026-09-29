
#pragma once

#define CRLF "\r\n"

#define RPL_CONNECTED(nickname) (":ircserv 001 " + nickname + " : Welcome to the IRC server!" + CRLF)

#define RPL_UMODEIS(hostname, channelname, mode, user)  (":" + hostname + " MODE " + channelname + " " + mode + " " + user + CRLF)

#define RPL_CREATIONTIME(nickname, channelname, creationtime) (":ircserv 329 " + nickname + " #" + channelname + " " + creationtime + CRLF)

#define RPL_CHANNELMODEIS(nickname, channelname, modes, values) (":ircserv 324 " + nickname + " #" + channelname + " " + modes + values + CRLF)

#define RPL_CHANGEMODE(hostname, channelname, mode, arguments) (":" + hostname + " MODE #" + channelname + " " + mode + " " + arguments + CRLF)

#define RPL_NICKCHANGE(oldnickname, nickname) (":" + oldnickname + " NICK " + nickname + CRLF)

#define RPL_JOINMSG(hostname, ipaddress, channelname) (":" + hostname + "@" + ipaddress + " JOIN #" + channelname + CRLF)

#define RPL_NAMREPLY(nickname, channelname, clientslist) (":ircserv 353 " + nickname + " @ #" + channelname + " :" + clientslist + CRLF)

#define RPL_ENDOFNAMES(nickname, channelname) (":ircserv 366 " + nickname + " #" + channelname + " :END of /NAMES list" + CRLF)

#define RPL_NOTOPIC(nick, channel) (":ircserv 331 " + nick + " #" + channel + " :No topic is set" + CRLF)

#define RPL_TOPICIS(nickname, channelname, topic) (":ircserv 332 " + nickname + " #" +channelname + " :" + topic + CRLF)


//-----------ERRORS----------------------------
#define ERR_NEEDMODEPARM(channelname, mode) (":ircserv 696 #" + channelname + " * You must specify a parameter for the key mode. " + mode + CRLF)

#define ERR_INVALIDMODEPARM(channelname, mode) (":ircserv 696 #" + channelname + " Invalid mode parameter. " + mode + CRLF)

#define ERR_KEYSET(channelname) (":ircserv 467 #" + channelname + " Channel key already set. " + CRLF)

#define ERR_UNKNOWNMODE(nickname, channelname, mode) (":ircserv 472 " + nickname + " #" + channelname + " " + mode + " :is not a recognised channel mode" + CRLF)

#define ERR_NOTENOUGHPARAM(nickname) (":ircserv 461 " + nickname + " :Not enough parameters." + CRLF)

#define ERR_CHANNELNOTFOUND(nickname, channelname) (":ircserv 403 " + nickname + " #" + channelname + " :No such channel" + CRLF)

#define ERR_CHANNELISFULL(nickname, channelname) (":ircserv 471 " + nickname + " #" + channelname + " :Cannot join channel (+l)" + CRLF)

#define ERR_INVITEONLYCHAN(client, channelname) (":ircserv 473 " + client + " #" + channelname + " :Cannot join channel (+i)")

#define ERR_BADCHANNELKEY(nickname, channelname) (":ircserv 475 " + nickname + " #" + channelname + " :Cannot join channel (+k)")

#define ERR_NOTOPERATOR(channelname) (":ircserv 482 #" + channelname + " :You're not a channel operator" + CRLF)

#define ERR_NOSUCHNICK(channelname, name) (":ircserv 401 #" + channelname + " " + name + " :No such nick/channel" + CRLF )

#define ERR_INCORPASS(nickname) (":ircserv 464 " + nickname + " :Password incorrect !" + CRLF )

#define ERR_ALREADYREGISTERED(nickname) (":ircserv 462 " + nickname + " :You may not reregister !" + CRLF )

#define ERR_NONICKNAME(nickname) (":ircserv 431 " + nickname + " :No nickname given" + CRLF )

#define ERR_NICKINUSE(nickname) (":ircserv 433 " + nickname + " :Nickname is already in use" + CRLF)

#define ERR_ERRONEUSNICK(nickname) (":ircserv 432 " + nickname + " :Erroneus nickname" + CRLF)

#define ERR_NOTREGISTERED(nickname) (":ircserv 451 " + nickname + " :You have not registered!" + CRLF)

#define ERR_CMDNOTFOUND(nickname, command) (":ircserv 421 " + nickname + " " + command + " :Unknown command" + CRLF)
