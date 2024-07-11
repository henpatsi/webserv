# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/07/10 11:39:42 by hpatsi            #+#    #+#              #
#    Updated: 2024/07/11 16:42:13 by hpatsi           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

CLASSES = HttpRequest

SOURCES = ${addsuffix .cpp, ${addprefix ./srcs/, main $(CLASSES)}}

OBJECTS = ${SOURCES:.cpp=.o}

FLAGS = -Wall -Wextra -Werror -std=c++11 -I ./includes

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

test:
	make
	./webserv confs/1.conf
