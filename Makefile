NAME = webserv

SRCS = server_utils.cpp main.cpp ServerManager.cpp response/Cgi.cpp response/Request.cpp response/Response.cpp parsing/parsing.cpp parsing/ParsingUtils.cpp

OBJS = $(addprefix obj/,$(SRCS:.cpp=.o))

CC = @c++
CFLAGS = -Wall -Werror -Wextra -std=c++11 -fsanitize=address

RM = @rm -f

obj/%.o : %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
	@echo "\033[1;32mWEBSERV COMPILED SUCCESSFULY\033[0m"

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)
	@rm -rf obj

re: fclean $(NAME)

.PHONY: all clean fclean re
