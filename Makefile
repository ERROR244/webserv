NAME = webServ

SRCS =  src/wrappers.cpp src/cgi/*.cpp \
		src/exceptions/*.cpp src/request/*.cpp src/response/*.cpp \
		src/server/*.cpp src/confi/*.cpp


CC = g++

CFLAGS = -I./includes -g3 #-fsanitize=address #-Wall -Wextra  #-Werror #-std=c++98

all : $(NAME)
	clear

$(NAME): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(NAME)


%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

fclean :
		@rm -rf $(NAME) $(CONFI)

re : fclean all
