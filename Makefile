# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/07/10 11:39:42 by hpatsi            #+#    #+#              #
#    Updated: 2024/07/10 11:44:03 by hpatsi           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

SOURCES = ${addsuffix .cpp, ${addprefix ./srcs/, main}}

OBJECTS = ${SOURCES:.cpp=.o}

#INCLUDES = 

FLAGS = -Wall -Wextra -Werror -std=c++11

COMP = c++ $(FLAGS)

all: $(NAME)

$(NAME): $(OBJECTS)
	$(COMP) $(OBJECTS) -o $(NAME)

%.o: %.cpp
	$(COMP) -c $< -o $@

clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(NAME)

re: fclean all
