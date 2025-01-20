NAME =  webServ

SRCS = src/srcClasses/*.cpp  src/srcCode/*.cpp

CC = g++

CFLAGS = -Wall -Wextra -Werror -I./includes -std=c++98 #-fsanitize=address -g3

all : $(NAME)

$(NAME): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

fclean :
		@rm -rf $(NAME)

re : fclean all