/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lanton-m <lanton-m@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 11:08:52 by dperez-p          #+#    #+#             */
/*   Updated: 2026/10/08 16:16:10 by lanton-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../inc/Server.hpp"
#include "../inc/Message.hpp"
#include "../inc/Client.hpp"
#include "../inc/Replies.hpp"
#include "../inc/Channel.hpp"

Server::Server()
{
	this->_serSocketFd = -1;
	this->_port = 0;
	_cmds["PASS"] = &Server::cmdPass;
	_cmds["NICK"] = &Server::cmdNick;
	_cmds["USER"] = &Server::cmdUser;
	_cmds["PRIVMSG"] = &Server::cmdPrivmsg;
	_cmds["QUIT"] = &Server::cmdQuit;
	_cmds["JOIN"] = &Server::cmdJoin;
	_cmds["PART"] = &Server::cmdPart;
	_cmds["TOPIC"] = &Server::cmdTopic;
	_cmds["MODE"] = &Server::cmdMode;
	_cmds["KICK"] = &Server::cmdKick;
	_cmds["INVITE"] = &Server::cmdInvite;
}

Server::~Server()
{}

Server::Server(Server const &oth)
{
	*this = oth;
}

Server &Server::operator=(Server const &oth)
{
	if (this != &oth)
	{
		this->_port = oth._port;
		this->_serSocketFd = oth._serSocketFd;
		this->_password = oth._password;
		this->_clients = oth._clients;
		this->_channel = oth._channel;
		this->_fds = oth._fds;
	}
	return (*this);
}

/********************************************** Getters *****************************************/
int	Server::getSerSocketFd() const
{
	return (_serSocketFd);
}

//Get client Fd from the server map
Client* Server::getClient(int fd)
{
	std::map<int, Client>::iterator it = _clients.find(fd);
	if (it != _clients.end())
	{
		return &(it->second);
	}
	return NULL;
}

std::string	Server::getPass() const
{
	return _password;
}

// static bool init
bool	Server::_signal = false;

void	Server::signalHandler(int signum)
{
	(void) signum;
	Server::_signal = true; // set signal to true to stop the server.
}

// close cleints from the server and the server socket.
void	Server::closeFds()
{
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		std::cout << "Client <" << it->second.getFd() << " Disconected." << std::endl;
		close(it->second.getFd());
	}
	if (_serSocketFd != -1) // close the server socket.
	{
		std::cout << "Server " << _serSocketFd << "Disconected." << std::endl;
		close(_serSocketFd);
	}
}

//clear clients
void	Server::clearClients(int fd)
{
	Client* client = getClient(fd);

	if (client)
	{
		for (size_t i = 0; i < _channel.size(); i++)
		{
			_channel[i].removeClient(*client);
		}
	}
	for (size_t i = 0; i < _fds.size(); i++) // remove the client from the pollfd
	{
		if (_fds[i].fd == fd)
		{
			_fds.erase(_fds.begin() + i);
			break ;
		}
	}
	_clients.erase(fd); // remove client from the map of clients
}

// accept new client, kernel checks for the process fd number empty
void	Server::acceptNewClient()
{
	Client	cli; // create client
	struct	sockaddr_in cliadd;
	struct	pollfd newPoll;
	socklen_t	len = sizeof(cliadd);

	int	incfd = accept(_serSocketFd, (sockaddr *)&(cliadd), &len); // accept the new client and register it to the kernel process
	if (incfd == -1)
	{
		std::cout << "accept() failed" << std::endl;
		return ;
	}
	if (fcntl(incfd, F_SETFL, O_NONBLOCK) == -1) // set the socket option for non-blocking socket
	{
		std::cout << "fcntl() failed" << std::endl;
		close(incfd); // release the kernel resource
		return ;
	}
	newPoll.fd = incfd; // add the client socket to the pollfd
	newPoll.events = POLLIN; // set the event to POLLIN for reading data
	newPoll.revents = 0; // set the revents to 0

	cli.setFd(incfd); // set the client file descriptor
	cli.setIpAdd(inet_ntoa((cliadd.sin_addr))); // convert the ipaddress to string and set
	_clients.insert(std::make_pair(incfd, cli)); // add client to the vector of clients
	_fds.push_back(newPoll); // add the client socket to the pollfdl

	std::cout << "Client <" << incfd << "> connection established" << std::endl;
}

void	Server::disconnectClient(int fd)
{
		std::cout << "Client " << fd << " disconnected." << std::endl;
		clearClients(fd); // clear the client
		close(fd);
}

void	Server::cmdPass(Client& client, const Message& msg)
{
	std::vector<std::string>	params = msg.getParam();

	if (client.getIsRegistered())
		return (send_msg(client, ERR_ALREADYREGISTERED(client.nickForReplay())));
	if (params.empty() || params[0] == "")	// u can ignore extra parameters
		return (send_msg(client, ERR_NOTENOUGHPARAM(client.nickForReplay())));
	if (params[0] == _password)
	{
		client.setLogged(true);
		tryRegister(client);
		return ;
	}
	return (send_msg(client, ERR_INCORPASS(client.nickForReplay())));
}

void	Server::tryRegister(Client& client)
{
	bool wasRegistered = client.getIsRegistered();

	client.setRegistered();
	if (!wasRegistered && client.getIsRegistered())
		send_msg(client, RPL_CONNECTED(client.nickForReplay()));
}
static bool specialchar(char c)
{
	std::string valids = "[]\\`^_{}|";

	return (valids.find(c) != std::string::npos);
}
bool	Server::nickInUse(std::string nick)
{
	for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (nick.size() != it->second.getNick().size())
			continue;
		bool equal = true;
		for (size_t i = 0; i < nick.size(); ++i)
		{
			if (std::tolower(static_cast<unsigned char>(nick[i])) !=
				std::tolower(static_cast<unsigned char>(it->second.getNick()[i])))
				equal = false;
		}
		if (equal)
			return true;
	}
	return false;
}
static bool	validNick(std::string nick)
{

	if (nick.length() > 9)
		return false;
	if (!isalpha(static_cast<unsigned char>(nick[0])) && !specialchar(nick[0]))
		return false;
	for (size_t i = 1; i < nick.length(); i++)
	{
		unsigned char character = static_cast<unsigned char>(nick[i]);
		if (!isalnum(character) && !specialchar(nick[i]) && nick[i] != '-')
			return false;
	}
	return true;
}
void	Server::cmdNick(Client& client, const Message& msg)
{
	std::vector<std::string> params = msg.getParam();

	if (params.empty() || params[0] == "")
		return (send_msg(client, ERR_NONICKNAME(client.nickForReplay())));
	if (!validNick(params[0]))
		return (send_msg(client, ERR_ERRONEUSNICK(client.nickForReplay())));
	if (params[0] == client.getNick())
		return ;
	if (nickInUse(params[0]))
		return (send_msg(client, ERR_NICKINUSE(client.nickForReplay())));
	bool wasRegistered = client.getIsRegistered();
	std::string oldPrefix = client.getPrefix();
	client.setNick(params[0]);
	tryRegister(client);
	if (wasRegistered)
		send_msg(client, RPL_NICKCHANGE(oldPrefix, client.getNick()));
}

void	Server::cmdUser(Client& client, const Message& msg)
{
	std::vector<std::string> params = msg.getParam();

	if (client.getIsRegistered())
		return (send_msg(client, ERR_ALREADYREGISTERED(client.nickForReplay())));
	if (params.size() < 4)
		return (send_msg(client, ERR_NOTENOUGHPARAM(client.nickForReplay())));
	client.setUser(params[0]);
	tryRegister(client);
}

std::vector<std::string> splitCommas(std::string params)
{
	std::vector<std::string>	targets;
	size_t 						pos = params.find(',');

	while (pos != std::string::npos)
	{
		targets.push_back(params.substr(0, pos));
		params.erase(0, pos + 1);
		pos = params.find(',');
	}
	if (!params.empty())
		targets.push_back(params);

	return targets;
}

bool	Server::fndUser(Client& client, std::vector<std::string>& params, std::string& target)
{
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second.getNick() == target)
		{
			send_msg(it->second, PRIVMSG_MESSAGE(client.getPrefix(), target, params[1]));
			return true;
		}
	}
	return false;

}

bool	Server::fndChannel(Client& client, std::vector<std::string>& params, std::string& target)
{
	std::string chan_name = target.erase(0,1);

	for (std::vector<Channel>::iterator it = _channel.begin(); it != _channel.end(); ++it)
	{
		if (it->getName() == chan_name)
		{
			if (!client.inChannel(*it))
				return false;
			it->broadcast(PRIVMSG_MESSAGE(client.getPrefix(), target, params[1]), &client);
			return true;
		}
	}
	return false;

}

void	Server::cmdPrivmsg(Client& client, const Message& msg)
{
	std::vector<std::string> params = msg.getParam(), targets;
	bool found;

	if (params.empty())
		return (send_msg(client, ERR_NORECIPIENT(client.nickForReplay(), msg.getCmd())));
	if (params.size() < 2 || params[1].empty())
		return (send_msg(client, ERR_NOTEXTTOSEND(client.nickForReplay())));
	targets = splitCommas(params[0]);
	for (size_t i = 0; i < targets.size(); i++)
	{
		found = false;
		if (targets[i][0] == '#')
			found = fndChannel(client, params, targets[i]);
		else
			found = fndUser(client, params, targets[i]);
		if (!found)
			send_msg(client, ERR_NOSUCHNICK(client.nickForReplay(), targets[i]));
	}
}

void	Server::cmdQuit(Client& client, const Message& msg)
{
	static_cast<void>(msg);
	disconnectClient(client.getFd());
}

void	Server::cmdJoin(Client& client, const Message& msg)
{
	std::vector<std::string>	params = msg.getParam(), channels, keys;
	std::string					act_key;
	bool						found;

	if (params.empty() || params[0].empty())
		return (send_msg(client, ERR_NOTENOUGHPARAM(client.nickForReplay())));
	channels = splitCommas(params[0]);
	if (params.size() > 1)
		keys = splitCommas(params[1]);
	for (size_t i = 0; i < channels.size(); i++)
	{
		found = false;
		if (channels[i].empty() || channels[i][0] != '#')
		{
			send_msg(client, ERR_CHANNELNOTFOUND(client.nickForReplay(), channels[i]));
			continue;
		}
		channels[i].erase(0, 1);
		for (size_t j = 0; j < _channel.size(); j++)
		{
			if (_channel[j].getName() == channels[i])
			{
				found = true;
				if (client.inChannel(_channel[j]))
					continue;
				(i >= keys.size()) ? act_key = "" : act_key = keys[i];
				_channel[j].addClient(client, act_key);
				break;
			}
		}
		if (!found)
		{
			(i >= keys.size()) ? act_key = "" : act_key = keys[i];
			_channel.push_back(Channel(channels[i], act_key));
			_channel.back().addClient(client, act_key);
		}
	}

}

void	Server::cmdPart(Client& client, const Message& msg)
{
	std::vector<std::string>	params = msg.getParam(), channels;
	std::string					text = "";
	bool						found;

	if (params.empty() || params[0].empty())
		return (send_msg(client, ERR_NOTENOUGHPARAM(client.nickForReplay())));
	channels = splitCommas(params[0]);
	if (params.size() > 1 && !params[1].empty())
		text = params[1];
	for (size_t i = 0; i < channels.size(); i++)
	{
		found = false;
		if (channels[i].empty() || channels[i][0] != '#')
		{
			send_msg(client, ERR_CHANNELNOTFOUND(client.nickForReplay(), channels[i]));
			continue;
		}
		channels[i].erase(0, 1);
		for (size_t j = 0; j < _channel.size(); j++)
		{
			if (channels[i] == _channel[j].getName())
			{
				found = true;
				_channel[j].part(client, text);
				break;
			}
		}
		if (!found)
			send_msg(client, ERR_CHANNELNOTFOUND(client.nickForReplay(), channels[i]));
	}
}

void	Server::cmdTopic(Client& client, const Message& msg)
{
	std::vector<std::string>	params = msg.getParam(), channel;
	std::string					text;
	bool						found = false;


	if (params.empty() || params[0].empty())
		return (send_msg(client, ERR_NOTENOUGHPARAM(client.nickForReplay())));
	channel = splitCommas(params[0]);
	if (channel.size() > 1 || channel[0].empty() || channel[0][0] != '#')
		return (send_msg(client, ERR_CHANNELNOTFOUND(client.nickForReplay(), params[0])));
	channel[0].erase(0, 1);
	(params.size() > 1 && !params[1].empty()) ? text = params[1] : text = "";
	for (size_t i = 0; i < _channel.size(); i++)
	{
		if (channel[0] == _channel[i].getName())
		{
			found = true;
			_channel[i].topic(client, text, params.size() == 1);
			break;
		}
	}
	if (!found)
		send_msg(client, ERR_CHANNELNOTFOUND(client.nickForReplay(), channel[0]));
}

void	Server::cmdMode(Client& client, const Message& msg)
{
	std::vector<std::string>	params = msg.getParam(), args;
	std::string					chan, modestr = "";
	bool						found = false;

	if (params.empty() || params[0].empty())
		return (send_msg(client, ERR_NOTENOUGHPARAM(client.nickForReplay())));
	if (params[0][0] != '#')
		return (send_msg(client, ERR_CHANNELNOTFOUND(client.nickForReplay(), params[0])));
	chan = params[0].erase(0, 1);
	for (size_t i = 0; i < _channel.size(); i++)
	{

		if (chan == _channel[i].getName())
		{
			found = true;
			if (params.size() == 1)
				_channel[i].showMode(client);
			else
			{
				if (!params[1].empty())
					modestr = params[1];
				if (params.size() > 2)
				{
					for (size_t j = 2; j < params.size(); j++)
						args.push_back(params[j]);
				}
				_channel[i].setMode(client, modestr, args);
			}
			break;
		}
	}
	if (!found)
		send_msg(client, ERR_CHANNELNOTFOUND(client.nickForReplay(), chan));
}

void	Server::cmdKick(Client& client, const Message& msg)
{
	std::vector<std::string>	params = msg.getParam();
	std::string					chan, nick, text = "";
	bool						chan_found = false, nick_found = false;

	if (params.empty() || params.size() < 2 || params[1].empty())
		return (send_msg(client, ERR_NOTENOUGHPARAM(client.nickForReplay())));
	if (params[0].empty() || params[0][0] != '#')
		return (send_msg(client, ERR_CHANNELNOTFOUND(client.nickForReplay(), params[0])));
	chan = params[0].erase(0,1);
	if (!params[1].empty())
		nick = params[1];
	if (params.size() >= 3)
		text = params[2];
	for (size_t i = 0; i < _channel.size(); i++)
	{
		if (chan == _channel[i].getName())
		{
			chan_found = true;
			for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
			{
				if (it->second.getNick() == nick)
				{
					nick_found = true;
					_channel[i].kick(client, it->second, text);
					break;
				}
			}
		}
	}
	if (!chan_found)
		return(send_msg(client, ERR_CHANNELNOTFOUND(client.nickForReplay(), chan)));
	if (!nick_found)
		return(send_msg(client, ERR_NOSUCHNICK(client.nickForReplay(), nick)));


}

void	Server::cmdInvite(Client& client, const Message& msg)
{
	std::vector<std::string>	params = msg.getParam();
	std::string					chan, nick;
	bool						chan_found = false, nick_found = false;

	if (params.empty() || params.size() < 2 || params[0].empty() || params[1].empty())
		return (send_msg(client, ERR_NOTENOUGHPARAM(client.nickForReplay())));
	if (params[1][0] != '#')
		return (send_msg(client, ERR_CHANNELNOTFOUND(client.nickForReplay(), params[1])));
	chan = params[1].erase(0, 1);
	nick = params[0];
	for (size_t i = 0; i < _channel.size(); i++)
	{
		if (chan == _channel[i].getName())
		{
			chan_found = true;
			for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
			{
				if (it->second.getNick() == nick)
				{
					nick_found = true;
					_channel[i].invite(client, it->second);
					break;
				}
			}
			break;
		}
	}
	if (!chan_found)
		return (send_msg(client, ERR_CHANNELNOTFOUND(client.nickForReplay(), chan)));
	if (!nick_found)
		return (send_msg(client, ERR_NOSUCHNICK(client.nickForReplay(), nick)));
}

static bool	accessCmd(std::string cmd)
{
	if (cmd == "PASS" || cmd == "NICK" || cmd == "USER" || cmd == "QUIT")
		return 1;
	return 0;
}

void	Server::executeCommand(Client& client, const Message& msg)
{
	std::map<std::string, CmdFunc>::iterator it;

	it = _cmds.find(msg.getCmd());
	if (it == _cmds.end())
		return (send_msg(client, ERR_CMDNOTFOUND(client.nickForReplay(), msg.getCmd())));
	if (!client.getIsRegistered() && !accessCmd(it->first))
		return (send_msg(client, ERR_NOTREGISTERED(client.nickForReplay())));
	(this->*(it->second))(client, msg);
}

// New data management
void	Server::recieveNewData(int fd)
{
	char	buff[1024]; // buffer for the data
	memset(buff, 0, sizeof(buff)); // clear the buffer
	Client* actualClient = getClient(fd);
	if (!actualClient) // if fd is not found
	{
		return ;
	}
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0); // recive the data

	if (bytes <= 0) // check if the client disconnected
	{
		disconnectClient(fd);
		return ;
	}
	else // print the recieved data
	{
		actualClient->appendBuffer(buff, bytes);
		std::vector<std::string> commands = actualClient->splitBuffer();
		if (actualClient->getIsOverSized())
		{
			disconnectClient(fd);
			return ;
		}
		for (size_t i = 0; i < commands.size(); i++)
		{
			Message Current(commands[i]);
			executeCommand(*actualClient, Current);
		}
	}
}

// server socket creation.
void	Server::serSocket()
{
	struct	sockaddr_in	add; // default sockeadd structure.
	struct	pollfd	newPoll; //default poll structure.
	add.sin_family = AF_INET; // set the address family to IPV4
	add.sin_port = htons(this->_port); // conver te port to network by order (big endian)
	add.sin_port = htons(this->_port); // conver te port to network by order (big endian)
	add.sin_addr.s_addr = INADDR_ANY; // set the adress to any Local machine address

	_serSocketFd = socket(AF_INET, SOCK_STREAM, 0); // creathe the server socket from the default socket function.
	if (_serSocketFd == -1) // check if fail
	{
		throw(std::runtime_error("faild to create socket."));
	}

	int	num = 1;
	if (setsockopt(_serSocketFd, SOL_SOCKET, SO_REUSEADDR, &num, sizeof(num)) == -1) // set the socket option (SO_REUSEADDR) to reuse the address with default setsockopt function skiping time_wait
	{
		throw(std::runtime_error("faild to set option SO_REUSERADDR on socket."));
	}
	if (fcntl(_serSocketFd, F_SETFL, O_NONBLOCK) == -1) // set the socket option (O_NONBLOCK) for not blocking socket with fcnlt default function
	{
		throw(std::runtime_error("faild to set O_NONBLOCK on socket."));
	}
	if (bind(_serSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1) // bind the socket to the address and port to enable other processes to communicate with it over the network
	{
		throw(std::runtime_error("faild to bind the socket to the adress."));
	}
	if (listen(_serSocketFd, SOMAXCONN) == -1) // listen for incoming connection and making the socket a passive socket
	{
		throw(std::runtime_error("listen() faild."));
	}

	newPoll.fd = _serSocketFd; // add the server socket to the poll.fd
	newPoll.events = POLLIN; // set the event to default POLLIN for reading
	newPoll.revents = 0; //set the revents to 0
	_fds.push_back(newPoll); // add the server socket structure to the pollfds;
}

// server init.
void	Server::serverInit(int port, const std::string password)
{
	this->_port = port;
	this->_password = password;
	serSocket(); // create the server socket

	std::cout << "Server: " << _serSocketFd << " connected." << std::endl;
	std::cout << "Waiting to accept a connection... \n";

	while (Server::_signal == false) // run the server until the signal is received
	{
		if ((poll(&_fds[0],_fds.size(), -1) == -1) && Server::_signal == false)
		{
			throw(std::runtime_error("poll() failed"));
		}
		for (size_t i = 0; i < _fds.size(); i++)
		{
			if (_fds[i].revents & POLLIN) // check if there is data to read
			{
				if (_fds[i].fd == _serSocketFd)
				{
					acceptNewClient(); // accept new client
				}
				else
				{
					size_t	sizePreData = _fds.size(); // control the size after the data.
					recieveNewData(_fds[i].fd);
					if (_fds.size() < sizePreData) // if one client was removed, don't skip the next fd
					{
						i--;
					}
				}
			}
		}
	}
	if (Server::_signal == true)
	{
		std::cout << std::endl << "Signal received correctly!" << std::endl;
	}
	closeFds(); // close the file descriptors when the server stops
}
