NAME = webServ
CPP = g++
INC = -I ./includes
SRC = $(wildcard src/*.cpp src/bstring/*.cpp src/cgi/*.cpp src/confi/*.cpp\
        src/exceptions/*.cpp src/parsers/*.cpp src/response/*.cpp src/server/*.cpp)
OBJ = $(SRC:.cpp=.o)
CPPFLAGS = -g3 -Wall -Wextra -Werror -std=c++98 #-fsanitize=address
%.o: %.cpp
	$(CPP) $(INC) $(CPPFLAGS) -c -o $@ $<

all: $(NAME)
# clear
# ./webServ confi.conf

$(NAME): $(OBJ)
	$(CPP) $(CPPFLAGS) $(OBJ) -o $(NAME)

clean:
	@rm -rf $(OBJ)

fclean: clean
	@rm -rf $(NAME)

re: fclean all

.PHONY: clean re all fclean

.SECONDARY: $(OBJ)