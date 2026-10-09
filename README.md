*This project has been created as part of the 42 curriculum by dperez-p, ramarti2, lanton-m.*

# ft_irc

## Description

`ft_irc` is an IRC server implemented in C++98 as part of the 42 curriculum.

The goal of this project is to create a functional Internet Relay Chat server that allows multiple clients to connect, register, exchange private messages and communicate through channels.

The server uses non-blocking TCP sockets and the `poll()` system call to manage multiple client connections. It includes client registration, IRC command parsing, channel management, operator privileges and message broadcasting.

## Instructions

### Requirements

- Linux or another Unix-like operating system
- C++98-compatible compiler
- `make`
- POSIX socket support

### Compilation

From the root of the repository, run:

```bash
make
```

The project is compiled using:

```text
-Wall -Wextra -Werror -std=c++98
```

Available Makefile commands:

```bash
make          # Compile the server
make clean    # Remove object files
make fclean   # Remove object files and the executable
make re       # Rebuild the project from scratch
```

### Execution

Start the server with:

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 42
```

The port must be between `1024` and `65535`.

The password:

- Must not be empty
- Must not contain spaces
- Must not be longer than 16 characters

### Connecting to the Server

You can connect using an IRC client or a command-line tool such as `netcat`:

```bash
nc 127.0.0.1 6667
```

A client must complete the following registration sequence:

```text
PASS 42
NICK alice
USER alice 0 * :Alice
```

The client is registered after providing a valid password, nickname and username.

### Supported Commands

#### Registration and Connection

- `PASS`
- `NICK`
- `USER`
- `QUIT`

#### Messaging

- `PRIVMSG`

#### Channel Management

- `JOIN`
- `PART`
- `TOPIC`
- `INVITE`
- `KICK`

#### Channel Modes

The following channel modes are supported:

- `+i`: Invite-only channel
- `+t`: Only channel operators can change the topic
- `+k`: Set a channel password
- `+o`: Grant or remove channel operator privileges
- `+l`: Set a maximum number of users

Examples:

```text
JOIN #general
PRIVMSG #general :Hello everyone
TOPIC #general :General discussion
MODE #general +it
MODE #general +k secret
MODE #general +l 10
```

### Project Structure

```text
inc/
	Channel.hpp
	Client.hpp
	Message.hpp
	Replies.hpp
	Server.hpp

src/
	Channel.cpp
	Client.cpp
	main.cpp
	Message.cpp
	Server.cpp
```

Main components:

- `main.cpp`: Program entry point and argument validation
- `Server.cpp`: Server socket, `poll()`, client management and command dispatching
- `Client.cpp`: Client state, registration and input buffering
- `Channel.cpp`: Channel members, operators, modes and message broadcasting
- `Message.cpp`: IRC command parsing
- `inc/`: Header files and IRC replies

## Resources

### IRC Documentation

- [RFC 1459: Internet Relay Chat Protocol](https://www.rfc-editor.org/rfc/rfc1459)
- [RFC 2810: IRC Architecture](https://www.rfc-editor.org/rfc/rfc2810)
- [RFC 2811: Internet Relay Chat Channel Management](https://www.rfc-editor.org/rfc/rfc2811)
- [RFC 2812: IRC Client Protocol](https://www.rfc-editor.org/rfc/rfc2812)
- [Modern IRC Documentation](https://modern.ircdocs.horse/)

### Network Programming

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [TCP Introduction](https://www.khanacademy.org/computing/computers-and-internet/xcae6f4a7ff015e7d/the-internet/v/transmission-control-protocol-tcp)
- [Listening vs. Connection Sockets](https://www.microsabio.net/dist/70rel/doc/ashref/listeningvs_connectionsockets.htm)

### Introduction to the Project

- [Introduction to ft_irc](https://medium.com/@afatir.ahmedfatir/small-irc-server-ft-irc-42-network-7cee848de6f9)
- [Socket Programming Overview](https://www.youtube.com/watch?v=NvZEZ-mZsuI&t=107s)

### AI Usage

AI tools were used as support during the development and documentation of the project for:

- Clarifying IRC protocol concepts and command behavior
- Reviewing socket management and client connection logic
- Reviewing client registration and input-buffer handling
- Improving the readability of the C++ implementation
- Documenting channel operations and channel modes

AI assistance was used mainly in the `Server`, `Client`, `Channel` and `Message` components, as well as in the project documentation. The final implementation and project decisions were reviewed by the authors.
