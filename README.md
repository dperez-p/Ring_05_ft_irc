*This project has been created as part of the 42 curriculum by dperez-p, ramarti2, lanton-m.*

# ft_irc

## Description

ft_irc is a C++ IRC server project developed as part of the 42 curriculum. The purpose of the project is to implement the core behavior of an Internet Relay Chat server, allowing multiple clients to connect, authenticate, exchange messages, and interact through channels, following the project subject and protocol basics.

The server is designed to handle client connections concurrently using non-blocking sockets and the polling mechanism. The goal is to recreate a simplified but functional IRC server, with the main focus on socket management, client state handling, command parsing, and channel communication.

This repository contains the implementation, source code, and project structure needed to compile and run the server locally.

## Instructions

### Requirements

- C++98 compatible compiler
- Linux environment
- POSIX socket libraries
- Make

### Compilation

From the root of the repository, run:

```bash
make
```

This will build the executable for the IRC server.

### Execution

Run the program with:

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 42
```

Then connect with any IRC client using the server host and port configured for the project.

### Notes

- The program must be compiled and run in a Unix-like environment.
- The implementation follows the 42 common standards for socket programming and C++ code quality.
- The project is intended to be evaluated by correct behavior against the IRC communication requirements described in the subject.

## Resources

### Classic references

- RFC 1459: Internet Relay Chat Protocol
- RFC 2810, RFC 2811, RFC 2812, RFC 2813: IRC protocol documentation
- Beej's Guide to Network Programming
- Official IRC protocol references and educational examples on client/server communication

### AI-assisted work

AI tools were used mainly as support for:

- clarifying IRC protocol concepts and command behavior
- reviewing the project structure and debugging logic related to sockets and client management
- improving code readability and helping with C++ implementation details
- drafting and refining the project documentation and README content

The AI was used to support the understanding, development, and documentation of the ft_irc project, especially in areas such as socket handling, client state logic, and the explanation of protocol-related behavior.
