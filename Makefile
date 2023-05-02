CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -fsanitize=address -g

SRCS_SRV	= server.cpp TCPIOEvent.cpp TCPListener.cpp KQueueTCPListener.cpp
OBJS_SRV	= $(SRCS_SRV:.cpp=.o)

SRCS_CLI	= client.cpp
OBJS_CLI	= $(SRCS_CLI:.cpp=.o)

all: server client

server: $(OBJS_SRV)
	$(CXX) $(CXXFLAGS) $(OBJS_SRV) -o $@ $(LDFLAGS)

client: $(OBJS_CLI)
	$(CXX) $(CXXFLAGS) $(OBJS_CLI) -o $@ $(LDFLAGS)

clean:
	$(RM) $(OBJS_SRV) $(OBJS_CLI)

fclean: clean
	$(RM) server client

re:
	make fclean
	make all

.PHONY: all clean fclean re
