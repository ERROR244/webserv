NAME = webServ
CPP = g++
INC = -I ./includes
SRC = $(wildcard src/*.cpp src/cgi/*.cpp src/exceptions/*.cpp src/confi/*.cpp\
        src/request/*.cpp src/response/*.cpp src/server/*.cpp src/bstring/*.cpp)
OBJ = $(SRC:.cpp=.o)
CPPFLAGS = -g3 -fsanitize=address #-std=c++98# -Wall -Wextra -Werror

%.o: %.cpp
	$(CPP) $(INC) $(CPPFLAGS) -c -o $@ $<

all: $(NAME)
	clear
	./webServ confi.conf

$(NAME): $(OBJ)
	$(CPP) $(CPPFLAGS) $(OBJ) -o $(NAME)

clean:
	@rm -rf $(OBJ)

fclean: clean
	@rm -rf $(NAME)

re: fclean all

.PHONY: clean re all fclean

.SECONDARY: $(OBJ)