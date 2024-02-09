CC = c++
CFLAGS = -Wall -Werror -Wextra -fsanitize=address
NAME = webserv
SRCS = server_utils.cpp main.cpp ServerManager.cpp ./response/Cgi.cpp ./response/Request.cpp ./response/Response.cpp ./parsing/parsing.cpp ./parsing/ParsingUtils.cpp
OBJS = $(SRCS:.cpp=.o)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean fclean all re
