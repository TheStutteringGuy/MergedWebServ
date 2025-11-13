# Variables :

CXX = c++
CPPFLAGS = -Wall -Wextra -Werror -std=c++98 -g3
RM = rm -f

NAME = webserv
SRC = main.cpp Parsing/*.cpp WebServ/*.cpp WebServ/*/*.cpp
HEADER = Parsing/*.hpp WebServ/*.hpp WebServ/*/*.hpp

# Rules :

all: $(NAME)

$(NAME) : $(SRC) $(HEADER)
	$(CXX) $(CPPFLAGS) $(SRC) -o $@

clean:

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
