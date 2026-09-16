# Ring_05_ft_irc

Minimal C++98 IRC server implementation for `ft_irc`.

## Build

```bash
make
```

## Run

```bash
./ircserv <port> <password>
```

## Implemented core behavior

- Non-blocking multi-client server using `poll`
- Registration flow: `PASS`, `NICK`, `USER`
- Core commands: `PING/PONG`, `JOIN`, `PART`, `PRIVMSG`, `QUIT`
- Channel features: `TOPIC`, `INVITE`, `KICK`, `MODE` (`i`, `t`, `k`, `o`, `l`)
- Common IRC numeric replies for error/success paths
