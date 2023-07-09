PROGRAM = webserver
SRC = src/functions/*.cpp
OBJ = src/functions/*.o
CC = c++
FLAGS = -std=c++98 -fsanitize=address -Wall -Wextra -Werror
C_FLAGS = $(CC) $(FLAGS)

all: $(PROGRAM)

$(PROGRAM): $(OBJ)
	$(C_FLAGS) $(OBJ) -o webserver

$(OBJ): $(SRC)
	cd src/functions && $(C_FLAGS) -c *.cpp

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(OBJ) $(PROGRAM)

re: fclean all

.PHONY: all clean fclean re