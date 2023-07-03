PROGRAM = webserver
SRC = src/fnc/*.cpp
OBJ = src/fnc/*.o
CC = c++
FLAGS = -std=c++11
C_FLAGS = $(CC) $(FLAGS)

all: $(PROGRAM)

$(PROGRAM): $(OBJ)
	$(C_FLAGS) $(OBJ) -o webserver

$(OBJ): $(SRC)
	cd src/fnc && $(C_FLAGS) -c *.cpp

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(OBJ) $(PROGRAM)

re: fclean all

.PHONY: all clean fclean re