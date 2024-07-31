# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/07/10 11:39:42 by hpatsi            #+#    #+#              #
#    Updated: 2024/07/16 09:41:29 by hpatsi           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME=webserv

########    UTILS    ########

NAMECOLOR	=	\033[1;36m
PIPECOLOR	=	\033[1;35m
FILECOLOR	=	\033[1;32m
OUTCOLOR	=	\033[1;34m
RESET		=	\033[0m
MESSAGE		=	\033[38;41m
INDENT		=	| sed 's/^/  /'

########    FILES    ########

SRCDIR		=	srcs/
OBJDIR		=	obj/
INCLUDEDIR	=	include/

FLAGS	=	-Wall -Wextra -Werror -std=c++17 \
			-I$(INCLUDEDIR) -g

SRC	=	$(wildcard $(SRCDIR)*.cpp)
OBJ	=	$(addprefix $(OBJDIR), $(notdir $(SRC:.cpp=.o)))
INC	=	$(wildcard $(INCLUDEDIR)*.hpp)

########    RULES    ########

all: $(NAME)


$(NAME): $(OBJDIR) $(OBJ) $(INC)
	@echo "$(NAMECOLOR)$(NAME) $(PIPECOLOR)| $(FILECOLOR)compiling executable: $(OUTCOLOR)$(NAME)$(RESET)"
	@c++ $(FLAGS) $(OBJ) -o $(NAME)

$(OBJDIR)%.o: $(SRCDIR)%.cpp
	@echo "$(NAMECOLOR)$(NAME) $(PIPECOLOR)| $(FILECOLOR)$< => $(OUTCOLOR)$@ $(RESET)"
	@c++ $(FLAGS) -c $< -o $@

$(OBJDIR):
	@mkdir -p $@

clean:
	@echo "$(MESSAGE)removing files:$(RESET)"
	@ls $(OBJDIR)* $(INDENT)
	@rm -rf $(OBJDIR)

fclean: clean
	@echo "$(MESSAGE)removing executable:$(RESET)"
	@echo "$(OUTCOLOR)$(NAME)$(RESET)" $(INDENT)
	@rm -rf $(NAME)

re: fclean all

########    TESTS    ########

test:
	make
	./webserv confs/1.conf


########    MISC.    ########
.PHONY: all clean fclean re
