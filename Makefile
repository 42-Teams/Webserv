CC = c++
CFLAGS = -Wall -Werror -Wextra -std=c++98
NAME = webserv
SRCS = server_utils.cpp main.cpp ServerManager.cpp ./response/Cgi.cpp ./response/Request.cpp ./response/Response.cpp ./parsing/parsing.cpp ./parsing/ParsingUtils.cpp
OBJS = $(patsubst %.cpp,obj/%.o,$(SRCS))

obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $^ -o $@
	@echo "\033[1;32mWEBSERV COMPILED SUCCESSFULY\033[0m"

clean:
	@rm -f $(OBJS)

fclean: clean
	@rm -f $(NAME)
	@rm -rf obj

re: fclean all

.PHONY: clean fclean all re