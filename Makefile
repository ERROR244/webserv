NAME =  webServ
CONFI = confi

SRCS = src/srcClasses/*.cpp  src/srcCode/*.cpp
SCONFI = src/srcClasses/confiClass.cpp src/srcClasses/confiClass_v2.cpp src/confiTest/confiTest.cpp src/srcCode/stringManipulation.cpp

CC = g++

CFLAGS = -Wall -Wextra -I./includes -fsanitize=address -w -g3 #-Werror #-std=c++98


all : $(NAME)

$(NAME): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(NAME)

CONFI: $(CONFI)

$(CONFI):  $(SCONFI)
	$(CC) $(CFLAGS) $(SCONFI) -o $(CONFI)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

fclean :
		@rm -rf $(NAME) $(CONFI)

re : fclean all