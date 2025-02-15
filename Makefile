NAME = webServ

SRCS =  src/wrappers.cpp src/cgi/*.cpp \
		src/exceptions/*.cpp src/request/*.cpp src/response/*.cpp \
		src/server/*.cpp src/confi/*.cpp


CC = g++

CFLAGS = -Wall -Wextra -I./includes -fsanitize=address -w -g3 #-Werror #-std=c++98

all : $(NAME)

$(NAME): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(NAME)


%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

fclean :
		@rm -rf $(NAME) $(CONFI)

re : fclean all