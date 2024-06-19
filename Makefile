#NAME------------------------------------------------------
NAME		=	server
#CMDS------------------------------------------------------
CXX			=	clang++
RM			=	rm -f
#FLAGS-----------------------------------------------------
CXXFLAGS	=	-Wall -Wextra
#FILES-----------------------------------------------------
OBJS		=	$(SRCS:.cpp=.o)
SRCS		=	main.cpp \
				Classes/Server.cpp \
				Classes/ServerEvent.cpp \
				Classes/MessageSendHandler.cpp


#KEY-------------------------------------------------------
all: $(NAME)

#NAME------------------------------------------------------
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

#OBJS------------------------------------------------------
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

#CLEAN-----------------------------------------------------
clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

#RE--------------------------------------------------------
re :
	$(MAKE) fclean
	$(MAKE) all

.PHONY: clean fclean all re