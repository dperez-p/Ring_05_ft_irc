# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: dperez-p <dperez-p@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/09/19 18:36:46 by dperez-p          #+#    #+#              #
#    Updated: 2026/10/08 10:13:35 by dperez-p         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv

CXX = c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98
CPPFLAGS	= -Iinc

SRCS = src/main.cpp \
	   src/Client.cpp \
	   src/Server.cpp \
	   src/Channel.cpp \
	   src/Message.cpp

OBJS = $(SRCS:src/%.cpp=objs/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

objs:
	@mkdir -p objs

objs/%.o: src/%.cpp | objs
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)

fclean: clean
	@rm -f $(NAME)
	@rm -rf objs

re: fclean all

.PHONY: all clean fclean re