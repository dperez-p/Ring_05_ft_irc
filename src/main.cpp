#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <set>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

namespace {

static volatile sig_atomic_t g_running = 1;

void signalHandler(int) {
    g_running = 0;
}

std::string toUpper(const std::string &s) {
    std::string out = s;
    for (size_t i = 0; i < out.size(); ++i) {
        out[i] = static_cast<char>(std::toupper(out[i]));
    }
    return out;
}

bool isValidNick(const std::string &nick) {
    if (nick.empty() || nick.size() > 30) {
        return false;
    }
    if (!std::isalpha(nick[0])) {
        return false;
    }
    for (size_t i = 0; i < nick.size(); ++i) {
        const char c = nick[i];
        if (!std::isalnum(c) && c != '-' && c != '_' && c != '[' && c != ']' && c != '\\' && c != '`' && c != '^' && c != '{' && c != '}') {
            return false;
        }
    }
    return true;
}

bool isValidChannel(const std::string &name) {
    return !name.empty() && name[0] == '#' && name.size() > 1 && name.find(' ') == std::string::npos;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> out;
    std::string cur;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == delim) {
            out.push_back(cur);
            cur.clear();
        } else {
            cur += s[i];
        }
    }
    out.push_back(cur);
    return out;
}

struct ParsedMessage {
    std::string command;
    std::vector<std::string> params;
};

ParsedMessage parseMessage(const std::string &line) {
    ParsedMessage msg;
    size_t i = 0;
    while (i < line.size() && line[i] == ' ') {
        ++i;
    }

    if (i < line.size() && line[i] == ':') {
        while (i < line.size() && line[i] != ' ') {
            ++i;
        }
    }

    while (i < line.size() && line[i] == ' ') {
        ++i;
    }

    while (i < line.size() && line[i] != ' ') {
        msg.command += line[i++];
    }

    while (i < line.size()) {
        while (i < line.size() && line[i] == ' ') {
            ++i;
        }
        if (i >= line.size()) {
            break;
        }
        if (line[i] == ':') {
            msg.params.push_back(line.substr(i + 1));
            break;
        }

        std::string token;
        while (i < line.size() && line[i] != ' ') {
            token += line[i++];
        }
        msg.params.push_back(token);
    }

    msg.command = toUpper(msg.command);
    return msg;
}

struct Client {
    int fd;
    std::string nick;
    std::string user;
    std::string realName;
    bool passOk;
    bool userSet;
    bool registered;
    std::string input;
    std::set<std::string> channels;

    Client() : fd(-1), passOk(false), userSet(false), registered(false) {}
};

struct Channel {
    std::string name;
    std::string topic;
    std::string key;
    bool inviteOnly;
    bool topicRestricted;
    size_t userLimit;
    std::set<int> members;
    std::set<int> operators;
    std::set<int> invited;

    Channel() : inviteOnly(false), topicRestricted(false), userLimit(0) {}
};

class Server {
public:
    Server(int port, const std::string &password)
        : _port(port), _password(password), _listenFd(-1), _name("irc.local") {}

    bool init() {
        _listenFd = socket(AF_INET, SOCK_STREAM, 0);
        if (_listenFd < 0) {
            std::cerr << "socket failed\n";
            return false;
        }

        int reuse = 1;
        if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            std::cerr << "setsockopt failed\n";
            return false;
        }

        if (!setNonBlocking(_listenFd)) {
            return false;
        }

        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(_port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(_listenFd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
            std::cerr << "bind failed\n";
            return false;
        }

        if (listen(_listenFd, SOMAXCONN) < 0) {
            std::cerr << "listen failed\n";
            return false;
        }

        pollfd pfd;
        pfd.fd = _listenFd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        _pollFds.push_back(pfd);
        return true;
    }

    void run() {
        while (g_running) {
            int ready = poll(&_pollFds[0], _pollFds.size(), 250);
            if (ready < 0) {
                if (errno == EINTR) {
                    continue;
                }
                break;
            }

            if (ready == 0) {
                continue;
            }

            for (size_t i = 0; i < _pollFds.size() && ready > 0; ++i) {
                if (_pollFds[i].revents == 0) {
                    continue;
                }
                --ready;

                if (_pollFds[i].fd == _listenFd && (_pollFds[i].revents & POLLIN)) {
                    acceptClients();
                    continue;
                }

                if (_pollFds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                    disconnect(_pollFds[i].fd, "Connection error");
                    i = 0;
                    continue;
                }

                if (_pollFds[i].revents & POLLIN) {
                    if (!readClient(_pollFds[i].fd)) {
                        i = 0;
                    }
                }
            }
        }

        shutdownAll();
    }

private:
    int _port;
    std::string _password;
    int _listenFd;
    std::string _name;
    std::vector<pollfd> _pollFds;
    std::map<int, Client> _clients;
    std::map<std::string, int> _nickToFd;
    std::map<std::string, Channel> _channels;

    static bool setNonBlocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0) {
            return false;
        }
        return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
    }

    void sendRaw(int fd, const std::string &msg) {
        const std::string out = msg + "\r\n";
        ssize_t sent = send(fd, out.c_str(), out.size(), MSG_NOSIGNAL);
        (void)sent;
    }

    std::string nickOrStar(const Client &client) const {
        return client.nick.empty() ? "*" : client.nick;
    }

    void sendNumeric(int fd, const std::string &code, const std::string &text) {
        Client &client = _clients[fd];
        sendRaw(fd, ":" + _name + " " + code + " " + nickOrStar(client) + " " + text);
    }

    std::string clientPrefix(int fd) {
        Client &client = _clients[fd];
        return ":" + client.nick + "!" + client.user + "@" + _name;
    }

    void acceptClients() {
        for (;;) {
            sockaddr_in addr;
            socklen_t len = sizeof(addr);
            int fd = accept(_listenFd, reinterpret_cast<sockaddr *>(&addr), &len);
            if (fd < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    break;
                }
                return;
            }

            if (!setNonBlocking(fd)) {
                close(fd);
                continue;
            }

            pollfd pfd;
            pfd.fd = fd;
            pfd.events = POLLIN;
            pfd.revents = 0;
            _pollFds.push_back(pfd);

            Client client;
            client.fd = fd;
            _clients[fd] = client;
        }
    }

    bool readClient(int fd) {
        char buf[512];
        for (;;) {
            ssize_t n = recv(fd, buf, sizeof(buf), 0);
            if (n < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    break;
                }
                disconnect(fd, "Read error");
                return false;
            }
            if (n == 0) {
                disconnect(fd, "Client quit");
                return false;
            }

            Client &client = _clients[fd];
            client.input.append(buf, static_cast<size_t>(n));

            size_t pos;
            while ((pos = client.input.find('\n')) != std::string::npos) {
                std::string line = client.input.substr(0, pos);
                client.input.erase(0, pos + 1);
                if (!line.empty() && line[line.size() - 1] == '\r') {
                    line.erase(line.size() - 1);
                }
                if (!line.empty()) {
                    handleLine(fd, line);
                    if (_clients.find(fd) == _clients.end()) {
                        return false;
                    }
                }
            }

            if (client.input.size() > 8192) {
                disconnect(fd, "Input too large");
                return false;
            }
        }

        return true;
    }

    void removePollFd(int fd) {
        for (std::vector<pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it) {
            if (it->fd == fd) {
                _pollFds.erase(it);
                return;
            }
        }
    }

    void leaveAllChannels(int fd, const std::string &reason) {
        Client &client = _clients[fd];
        std::vector<std::string> chans(client.channels.begin(), client.channels.end());
        for (size_t i = 0; i < chans.size(); ++i) {
            if (_channels.find(chans[i]) == _channels.end()) {
                continue;
            }

            Channel &ch = _channels[chans[i]];
            broadcastToChannel(chans[i], clientPrefix(fd) + " QUIT :" + reason, fd);
            ch.members.erase(fd);
            ch.operators.erase(fd);
            ch.invited.erase(fd);
            if (ch.members.empty()) {
                _channels.erase(chans[i]);
            }
        }
        client.channels.clear();
    }

    void disconnect(int fd, const std::string &reason) {
        if (_clients.find(fd) == _clients.end()) {
            return;
        }

        Client &client = _clients[fd];
        if (client.registered) {
            leaveAllChannels(fd, reason);
        }

        if (!client.nick.empty()) {
            _nickToFd.erase(client.nick);
        }

        close(fd);
        _clients.erase(fd);
        removePollFd(fd);
    }

    void maybeRegister(int fd) {
        Client &client = _clients[fd];
        if (client.registered || !client.passOk || client.nick.empty() || !client.userSet) {
            return;
        }

        client.registered = true;
        sendNumeric(fd, "001", ":Welcome to IRC, " + client.nick);
        sendNumeric(fd, "002", ":Your host is " + _name);
    }

    bool requireRegistered(int fd) {
        if (_clients[fd].registered) {
            return true;
        }
        sendNumeric(fd, "451", ":You have not registered");
        return false;
    }

    void handlePass(int fd, const ParsedMessage &msg) {
        Client &client = _clients[fd];
        if (client.registered) {
            sendNumeric(fd, "462", ":You may not reregister");
            return;
        }

        if (msg.params.empty()) {
            sendNumeric(fd, "461", "PASS :Not enough parameters");
            return;
        }

        if (msg.params[0] != _password) {
            sendNumeric(fd, "464", ":Password incorrect");
            return;
        }

        client.passOk = true;
        maybeRegister(fd);
    }

    void handleNick(int fd, const ParsedMessage &msg) {
        Client &client = _clients[fd];
        if (msg.params.empty()) {
            sendNumeric(fd, "431", ":No nickname given");
            return;
        }

        const std::string nextNick = msg.params[0];
        if (!isValidNick(nextNick)) {
            sendNumeric(fd, "432", nextNick + " :Erroneous nickname");
            return;
        }

        std::map<std::string, int>::iterator it = _nickToFd.find(nextNick);
        if (it != _nickToFd.end() && it->second != fd) {
            sendNumeric(fd, "433", nextNick + " :Nickname is already in use");
            return;
        }

        std::string oldPrefix;
        if (client.registered) {
            oldPrefix = clientPrefix(fd);
        }

        if (!client.nick.empty()) {
            _nickToFd.erase(client.nick);
        }
        client.nick = nextNick;
        _nickToFd[nextNick] = fd;

        if (!oldPrefix.empty()) {
            sendRaw(fd, oldPrefix + " NICK :" + nextNick);
            for (std::set<std::string>::const_iterator cit = client.channels.begin(); cit != client.channels.end(); ++cit) {
                broadcastToChannel(*cit, oldPrefix + " NICK :" + nextNick, fd);
            }
        }

        maybeRegister(fd);
    }

    void handleUser(int fd, const ParsedMessage &msg) {
        Client &client = _clients[fd];
        if (client.userSet) {
            sendNumeric(fd, "462", ":You may not reregister");
            return;
        }

        if (msg.params.size() < 4) {
            sendNumeric(fd, "461", "USER :Not enough parameters");
            return;
        }

        client.user = msg.params[0];
        client.realName = msg.params[3];
        client.userSet = true;
        maybeRegister(fd);
    }

    bool ensureInChannel(int fd, const std::string &channelName) {
        if (_channels.find(channelName) == _channels.end() || _channels[channelName].members.find(fd) == _channels[channelName].members.end()) {
            sendNumeric(fd, "442", channelName + " :You're not on that channel");
            return false;
        }
        return true;
    }

    void broadcastToChannel(const std::string &channelName, const std::string &line, int exceptFd) {
        if (_channels.find(channelName) == _channels.end()) {
            return;
        }

        Channel &ch = _channels[channelName];
        for (std::set<int>::const_iterator it = ch.members.begin(); it != ch.members.end(); ++it) {
            if (*it != exceptFd) {
                sendRaw(*it, line);
            }
        }
    }

    void sendNames(int fd, const std::string &channelName) {
        Channel &ch = _channels[channelName];
        std::string names;
        for (std::set<int>::const_iterator it = ch.members.begin(); it != ch.members.end(); ++it) {
            Client &member = _clients[*it];
            if (!names.empty()) {
                names += " ";
            }
            if (ch.operators.find(*it) != ch.operators.end()) {
                names += "@";
            }
            names += member.nick;
        }
        sendNumeric(fd, "353", "= " + channelName + " :" + names);
        sendNumeric(fd, "366", channelName + " :End of /NAMES list");
    }

    void joinSingle(int fd, const std::string &channelName, const std::string &key) {
        Client &client = _clients[fd];
        if (!isValidChannel(channelName)) {
            sendNumeric(fd, "403", channelName + " :No such channel");
            return;
        }

        Channel &ch = _channels[channelName];
        if (ch.name.empty()) {
            ch.name = channelName;
        }

        if (ch.members.find(fd) != ch.members.end()) {
            return;
        }

        if (ch.inviteOnly && ch.invited.find(fd) == ch.invited.end()) {
            sendNumeric(fd, "473", channelName + " :Cannot join channel (+i)");
            return;
        }

        if (!ch.key.empty() && ch.key != key) {
            sendNumeric(fd, "475", channelName + " :Cannot join channel (+k)");
            return;
        }

        if (ch.userLimit > 0 && ch.members.size() >= ch.userLimit) {
            sendNumeric(fd, "471", channelName + " :Cannot join channel (+l)");
            return;
        }

        ch.members.insert(fd);
        ch.invited.erase(fd);
        client.channels.insert(channelName);
        if (ch.operators.empty()) {
            ch.operators.insert(fd);
        }

        const std::string line = clientPrefix(fd) + " JOIN " + channelName;
        sendRaw(fd, line);
        broadcastToChannel(channelName, line, fd);

        if (!ch.topic.empty()) {
            sendNumeric(fd, "332", channelName + " :" + ch.topic);
        }
        sendNames(fd, channelName);
    }

    void handleJoin(int fd, const ParsedMessage &msg) {
        if (msg.params.empty()) {
            sendNumeric(fd, "461", "JOIN :Not enough parameters");
            return;
        }

        std::vector<std::string> channels = split(msg.params[0], ',');
        std::vector<std::string> keys;
        if (msg.params.size() > 1) {
            keys = split(msg.params[1], ',');
        }

        for (size_t i = 0; i < channels.size(); ++i) {
            const std::string key = i < keys.size() ? keys[i] : "";
            joinSingle(fd, channels[i], key);
        }
    }

    void handlePart(int fd, const ParsedMessage &msg) {
        if (msg.params.empty()) {
            sendNumeric(fd, "461", "PART :Not enough parameters");
            return;
        }

        std::vector<std::string> channels = split(msg.params[0], ',');
        const std::string reason = msg.params.size() > 1 ? msg.params[1] : _clients[fd].nick;

        for (size_t i = 0; i < channels.size(); ++i) {
            if (_channels.find(channels[i]) == _channels.end()) {
                sendNumeric(fd, "403", channels[i] + " :No such channel");
                continue;
            }
            if (!ensureInChannel(fd, channels[i])) {
                continue;
            }

            Channel &ch = _channels[channels[i]];
            const std::string line = clientPrefix(fd) + " PART " + channels[i] + " :" + reason;
            sendRaw(fd, line);
            broadcastToChannel(channels[i], line, fd);

            ch.members.erase(fd);
            ch.operators.erase(fd);
            ch.invited.erase(fd);
            _clients[fd].channels.erase(channels[i]);
            if (ch.members.empty()) {
                _channels.erase(channels[i]);
            }
        }
    }

    void handlePrivMsg(int fd, const ParsedMessage &msg) {
        if (msg.params.empty()) {
            sendNumeric(fd, "411", ":No recipient given (PRIVMSG)");
            return;
        }
        if (msg.params.size() < 2 || msg.params[1].empty()) {
            sendNumeric(fd, "412", ":No text to send");
            return;
        }

        const std::string &target = msg.params[0];
        const std::string line = clientPrefix(fd) + " PRIVMSG " + target + " :" + msg.params[1];

        if (!target.empty() && target[0] == '#') {
            if (_channels.find(target) == _channels.end()) {
                sendNumeric(fd, "401", target + " :No such nick/channel");
                return;
            }
            if (!ensureInChannel(fd, target)) {
                return;
            }
            broadcastToChannel(target, line, fd);
            return;
        }

        std::map<std::string, int>::iterator it = _nickToFd.find(target);
        if (it == _nickToFd.end()) {
            sendNumeric(fd, "401", target + " :No such nick/channel");
            return;
        }
        sendRaw(it->second, line);
    }

    void handleTopic(int fd, const ParsedMessage &msg) {
        if (msg.params.empty()) {
            sendNumeric(fd, "461", "TOPIC :Not enough parameters");
            return;
        }

        const std::string &channelName = msg.params[0];
        if (_channels.find(channelName) == _channels.end()) {
            sendNumeric(fd, "403", channelName + " :No such channel");
            return;
        }
        if (!ensureInChannel(fd, channelName)) {
            return;
        }

        Channel &ch = _channels[channelName];

        if (msg.params.size() == 1) {
            if (ch.topic.empty()) {
                sendNumeric(fd, "331", channelName + " :No topic is set");
            } else {
                sendNumeric(fd, "332", channelName + " :" + ch.topic);
            }
            return;
        }

        if (ch.topicRestricted && ch.operators.find(fd) == ch.operators.end()) {
            sendNumeric(fd, "482", channelName + " :You're not channel operator");
            return;
        }

        ch.topic = msg.params[1];
        const std::string line = clientPrefix(fd) + " TOPIC " + channelName + " :" + ch.topic;
        sendRaw(fd, line);
        broadcastToChannel(channelName, line, fd);
    }

    void handleInvite(int fd, const ParsedMessage &msg) {
        if (msg.params.size() < 2) {
            sendNumeric(fd, "461", "INVITE :Not enough parameters");
            return;
        }

        const std::string &targetNick = msg.params[0];
        const std::string &channelName = msg.params[1];
        if (_channels.find(channelName) == _channels.end()) {
            sendNumeric(fd, "403", channelName + " :No such channel");
            return;
        }
        if (!ensureInChannel(fd, channelName)) {
            return;
        }

        Channel &ch = _channels[channelName];
        if (ch.operators.find(fd) == ch.operators.end()) {
            sendNumeric(fd, "482", channelName + " :You're not channel operator");
            return;
        }

        std::map<std::string, int>::iterator it = _nickToFd.find(targetNick);
        if (it == _nickToFd.end()) {
            sendNumeric(fd, "401", targetNick + " :No such nick/channel");
            return;
        }

        if (ch.members.find(it->second) != ch.members.end()) {
            sendNumeric(fd, "443", targetNick + " " + channelName + " :is already on channel");
            return;
        }

        ch.invited.insert(it->second);
        sendNumeric(fd, "341", targetNick + " " + channelName);
        sendRaw(it->second, clientPrefix(fd) + " INVITE " + targetNick + " :" + channelName);
    }

    void handleKick(int fd, const ParsedMessage &msg) {
        if (msg.params.size() < 2) {
            sendNumeric(fd, "461", "KICK :Not enough parameters");
            return;
        }

        const std::string &channelName = msg.params[0];
        const std::string &targetNick = msg.params[1];
        const std::string reason = msg.params.size() > 2 ? msg.params[2] : targetNick;

        if (_channels.find(channelName) == _channels.end()) {
            sendNumeric(fd, "403", channelName + " :No such channel");
            return;
        }
        if (!ensureInChannel(fd, channelName)) {
            return;
        }

        Channel &ch = _channels[channelName];
        if (ch.operators.find(fd) == ch.operators.end()) {
            sendNumeric(fd, "482", channelName + " :You're not channel operator");
            return;
        }

        std::map<std::string, int>::iterator targetIt = _nickToFd.find(targetNick);
        if (targetIt == _nickToFd.end() || ch.members.find(targetIt->second) == ch.members.end()) {
            sendNumeric(fd, "441", targetNick + " " + channelName + " :They aren't on that channel");
            return;
        }

        const int targetFd = targetIt->second;
        const std::string line = clientPrefix(fd) + " KICK " + channelName + " " + targetNick + " :" + reason;
        sendRaw(fd, line);
        broadcastToChannel(channelName, line, fd);
        sendRaw(targetFd, line);

        ch.members.erase(targetFd);
        ch.operators.erase(targetFd);
        ch.invited.erase(targetFd);
        _clients[targetFd].channels.erase(channelName);
        if (ch.members.empty()) {
            _channels.erase(channelName);
        }
    }

    void handleMode(int fd, const ParsedMessage &msg) {
        if (msg.params.empty()) {
            sendNumeric(fd, "461", "MODE :Not enough parameters");
            return;
        }

        const std::string &target = msg.params[0];
        if (target.empty() || target[0] != '#') {
            sendNumeric(fd, "501", ":Unknown MODE flag");
            return;
        }

        if (_channels.find(target) == _channels.end()) {
            sendNumeric(fd, "403", target + " :No such channel");
            return;
        }

        Channel &ch = _channels[target];
        if (msg.params.size() == 1) {
            std::string modes = "+";
            if (ch.inviteOnly) {
                modes += "i";
            }
            if (ch.topicRestricted) {
                modes += "t";
            }
            if (!ch.key.empty()) {
                modes += "k";
            }
            if (ch.userLimit > 0) {
                modes += "l";
            }
            sendNumeric(fd, "324", target + " " + modes);
            return;
        }

        if (!ensureInChannel(fd, target)) {
            return;
        }
        if (ch.operators.find(fd) == ch.operators.end()) {
            sendNumeric(fd, "482", target + " :You're not channel operator");
            return;
        }

        const std::string modeChanges = msg.params[1];
        bool adding = true;
        size_t argIndex = 2;
        std::string appliedModes;
        std::vector<std::string> appliedArgs;

        for (size_t i = 0; i < modeChanges.size(); ++i) {
            const char m = modeChanges[i];
            if (m == '+') {
                adding = true;
                if (appliedModes.empty() || appliedModes[appliedModes.size() - 1] != '+') {
                    appliedModes += '+';
                }
                continue;
            }
            if (m == '-') {
                adding = false;
                if (appliedModes.empty() || appliedModes[appliedModes.size() - 1] != '-') {
                    appliedModes += '-';
                }
                continue;
            }

            if (appliedModes.empty()) {
                appliedModes += adding ? '+' : '-';
            } else {
                char last = appliedModes[appliedModes.size() - 1];
                if (last == '+' || last == '-') {
                    // keep current sign
                } else {
                    const char sign = adding ? '+' : '-';
                    size_t j = appliedModes.size();
                    while (j > 0 && appliedModes[j - 1] != '+' && appliedModes[j - 1] != '-') {
                        --j;
                    }
                    if (j == 0 || appliedModes[j - 1] != sign) {
                        appliedModes += sign;
                    }
                }
            }

            switch (m) {
                case 'i':
                    ch.inviteOnly = adding;
                    appliedModes += m;
                    break;
                case 't':
                    ch.topicRestricted = adding;
                    appliedModes += m;
                    break;
                case 'k':
                    if (adding) {
                        if (argIndex >= msg.params.size()) {
                            continue;
                        }
                        ch.key = msg.params[argIndex++];
                        appliedModes += m;
                        appliedArgs.push_back(ch.key);
                    } else {
                        ch.key.clear();
                        appliedModes += m;
                    }
                    break;
                case 'l':
                    if (adding) {
                        if (argIndex >= msg.params.size()) {
                            continue;
                        }
                        size_t limit = static_cast<size_t>(std::atoi(msg.params[argIndex++].c_str()));
                        if (limit == 0) {
                            continue;
                        }
                        ch.userLimit = limit;
                        appliedModes += m;
                        std::ostringstream oss;
                        oss << limit;
                        appliedArgs.push_back(oss.str());
                    } else {
                        ch.userLimit = 0;
                        appliedModes += m;
                    }
                    break;
                case 'o':
                    if (argIndex >= msg.params.size()) {
                        continue;
                    }
                    {
                        const std::string nick = msg.params[argIndex++];
                        std::map<std::string, int>::iterator it = _nickToFd.find(nick);
                        if (it == _nickToFd.end() || ch.members.find(it->second) == ch.members.end()) {
                            sendNumeric(fd, "441", nick + " " + target + " :They aren't on that channel");
                            continue;
                        }
                        if (adding) {
                            ch.operators.insert(it->second);
                        } else {
                            ch.operators.erase(it->second);
                        }
                        appliedModes += m;
                        appliedArgs.push_back(nick);
                    }
                    break;
                default:
                    sendNumeric(fd, "472", std::string(1, m) + " :is unknown mode char to me");
                    break;
            }
        }

        if (appliedModes.empty()) {
            return;
        }

        std::string line = clientPrefix(fd) + " MODE " + target + " " + appliedModes;
        for (size_t i = 0; i < appliedArgs.size(); ++i) {
            line += " " + appliedArgs[i];
        }

        sendRaw(fd, line);
        broadcastToChannel(target, line, fd);
    }

    void handlePing(int fd, const ParsedMessage &msg) {
        if (msg.params.empty()) {
            sendNumeric(fd, "409", ":No origin specified");
            return;
        }
        sendRaw(fd, ":" + _name + " PONG " + _name + " :" + msg.params[0]);
    }

    void handleQuit(int fd, const ParsedMessage &msg) {
        const std::string reason = msg.params.empty() ? "Quit" : msg.params[0];
        if (_clients[fd].registered) {
            const std::string line = clientPrefix(fd) + " QUIT :" + reason;
            for (std::set<std::string>::const_iterator it = _clients[fd].channels.begin(); it != _clients[fd].channels.end(); ++it) {
                broadcastToChannel(*it, line, fd);
            }
        }
        disconnect(fd, reason);
    }

    void handleLine(int fd, const std::string &line) {
        ParsedMessage msg = parseMessage(line);
        if (msg.command.empty()) {
            return;
        }

        if (msg.command == "PASS") {
            handlePass(fd, msg);
            return;
        }
        if (msg.command == "NICK") {
            handleNick(fd, msg);
            return;
        }
        if (msg.command == "USER") {
            handleUser(fd, msg);
            return;
        }
        if (msg.command == "PING") {
            handlePing(fd, msg);
            return;
        }
        if (msg.command == "QUIT") {
            handleQuit(fd, msg);
            return;
        }

        if (!requireRegistered(fd)) {
            return;
        }

        if (msg.command == "PONG") {
            return;
        }
        if (msg.command == "JOIN") {
            handleJoin(fd, msg);
        } else if (msg.command == "PART") {
            handlePart(fd, msg);
        } else if (msg.command == "PRIVMSG") {
            handlePrivMsg(fd, msg);
        } else if (msg.command == "TOPIC") {
            handleTopic(fd, msg);
        } else if (msg.command == "INVITE") {
            handleInvite(fd, msg);
        } else if (msg.command == "KICK") {
            handleKick(fd, msg);
        } else if (msg.command == "MODE") {
            handleMode(fd, msg);
        } else {
            sendNumeric(fd, "421", msg.command + " :Unknown command");
        }
    }

    void shutdownAll() {
        std::vector<int> fds;
        for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
            fds.push_back(it->first);
        }
        for (size_t i = 0; i < fds.size(); ++i) {
            disconnect(fds[i], "Server shutdown");
        }

        if (_listenFd >= 0) {
            close(_listenFd);
            _listenFd = -1;
        }
    }
};

bool parsePort(const char *s, int &port) {
    char *end = NULL;
    long p = std::strtol(s, &end, 10);
    if (*s == '\0' || *end != '\0' || p < 1 || p > 65535) {
        return false;
    }
    port = static_cast<int>(p);
    return true;
}

}  // namespace

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>\n";
        return 1;
    }

    int port = 0;
    if (!parsePort(argv[1], port) || std::string(argv[2]).empty()) {
        std::cerr << "Invalid arguments\n";
        return 1;
    }

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    Server server(port, argv[2]);
    if (!server.init()) {
        return 1;
    }

    server.run();
    return 0;
}
