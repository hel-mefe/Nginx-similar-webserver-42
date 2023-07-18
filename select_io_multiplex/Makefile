# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: fchanane <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/07/15 22:00:59 by fchanane          #+#    #+#              #
#    Updated: 2023/07/15 22:01:03 by fchanane         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= webserv

OBJ = $(SRC:.cpp=.o)

CC = c++  

CPPFLAGS	=  -Wall -Wextra -Werror -std=c++98

RM		=	rm -f

SRC = *.cpp


$(NAME): $(OBJ)
	$(CC) $(CPPFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

all: $(NAME)

clean:
	@$(RM) $(OBJ)

fclean: clean
	@$(RM) $(NAME)

re: fclean all
