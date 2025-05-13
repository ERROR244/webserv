NAME = webServ
CPP = c++
INC = -I ./includes
SRC = $(wildcard src/*.cpp src/bstring/*.cpp src/cgi/*.cpp src/confi/*.cpp\
        src/exceptions/*.cpp src/parsers/*.cpp src/response/*.cpp src/server/*.cpp)
H_SRC = $(wildcard includes/*.hpp includes/*.h)
OBJ = $(SRC:.cpp=.o)
CPPFLAGS = -Wall -Wextra -Werror -std=c++98

%.o: %.cpp
	$(CPP) $(INC) $(CPPFLAGS) -c -o $@ $<

all: $(NAME)

$(NAME): $(OBJ) $(H_SRC)
	$(CPP) $(CPPFLAGS) $(OBJ) -o $(NAME)

clean:
	@rm -rf $(OBJ)

fclean: clean
	@rm -rf $(NAME)

re: fclean all

.PHONY: clean re all fclean

.SECONDARY: $(OBJ)