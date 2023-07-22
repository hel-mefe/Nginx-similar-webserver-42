PROGRAM = webserver

CC = c++
FLAGS = -std=c++98 -fsanitize=address -Wall -Wextra -Werror
C_FLAGS = $(CC) $(FLAGS)

HDLS = src/handlers/
MTDS = src/methods/
MLXS = src/multiplixers/
PRS = src/parsers/
TLS = src/tools/

all: $(PROGRAM)

$(PROGRAM): $(HDLS)*.o $(MTDS)*.o $(MLXS)*.o $(PRS)*.o $(TLS)*.o
	$(C_FLAGS) $(HDLS)*.o $(MTDS)*.o $(MLXS)*.o $(PRS)*.o $(TLS)*.o -o webserver

$(HDLS)*.o: $(HDLS)*.cpp
	cd $(HDLS) && $(C_FLAGS) -c *.cpp

$(MTDS)*.o: $(MTDS)*.cpp
	cd $(MTDS) && $(C_FLAGS) -c *.cpp

$(MLXS)*.o: $(MLXS)*.cpp
	cd $(MLXS) && $(C_FLAGS) -c *.cpp

$(PRS)*.o: $(PRS)*.cpp
	cd $(PRS) && $(C_FLAGS) -c *.cpp

$(TLS)*.o: $(TLS)*.cpp
	cd $(TLS) && $(C_FLAGS) -c *.cpp

clean:
	rm -f $(HDLS)*.o $(MTDS)*.o $(MLXS)*.o $(PRS)*.o $(TLS)*.o

fclean: clean
	rm -f $(HDLS)*.o $(MTDS)*.o $(MLXS)*.o $(PRS)*.o $(TLS)*.o $(PROGRAM)

re: fclean all

.PHONY: all clean fclean re
