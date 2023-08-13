PROGRAM = webserv

CC = c++
FLAGS = -std=c++98 -Wall -Wextra -Werror
C_FLAGS = $(CC) $(FLAGS)

HDLS = src/handlers/handlers.methods.cpp src/handlers/http_handler.methods.cpp src/handlers/webserver.methods.cpp
MTDS = src/methods/delete.method.cpp src/methods/get.method.cpp src/methods/options.method.cpp src/methods/post.method.cpp src/methods/put.method.cpp src/methods/trace.method.cpp
MLXS = src/multiplixers/kqueue.multiplixer.cpp
PRS = src/parsers/http.parser.cpp src/parsers/methods.parser.cpp src/parsers/tokenschecker.parser.cpp
TLS = src/tools/cgi.tools.cpp src/tools/cli.tools.cpp src/tools/main.cpp src/tools/random.tools.cpp src/tools/server.tools.cpp src/tools/upload.tools.cpp

F_HDLS = handlers.methods.cpp http_handler.methods.cpp webserver.methods.cpp
F_MTDS = delete.method.cpp get.method.cpp options.method.cpp post.method.cpp put.method.cpp trace.method.cpp
F_MLXS = kqueue.multiplixer.cpp
F_PRS = http.parser.cpp methods.parser.cpp tokenschecker.parser.cpp
F_TLS = cgi.tools.cpp cli.tools.cpp main.cpp random.tools.cpp server.tools.cpp upload.tools.cpp

all: $(PROGRAM)

$(PROGRAM): $(HDLS:.cpp=.o) $(MTDS:.cpp=.o) $(MLXS:.cpp=.o) $(PRS:.cpp=.o) $(TLS:.cpp=.o)
	$(C_FLAGS) $(HDLS:.cpp=.o) $(MTDS:.cpp=.o) $(MLXS:.cpp=.o) $(PRS:.cpp=.o) $(TLS:.cpp=.o) -o $(PROGRAM)

$(HDLS:.cpp=.o): $(HDLS)
	cd src/handlers && $(C_FLAGS) -c $(F_HDLS)

$(MTDS:.cpp=.o): $(MTDS)
	cd src/methods && $(C_FLAGS) -c $(F_MTDS)

$(MLXS:.cpp=.o): $(MLXS)
	cd src/multiplixers && $(C_FLAGS) -c $(F_MLXS)

$(PRS:.cpp=.o): $(PRS)
	cd src/parsers && $(C_FLAGS) -c $(F_PRS)

$(TLS:.cpp=.o): $(TLS)
	cd src/tools && $(C_FLAGS) -c $(F_TLS)

clean:
	rm -f $(HDLS:.cpp=.o) $(MTDS:.cpp=.o) $(MLXS:.cpp=.o) $(PRS:.cpp=.o) $(TLS:.cpp=.o)

fclean: clean
	rm -f $(PROGRAM)

re: fclean all

.PHONY: all clean fclean re