NAME = WebServ

CC = c++
INCLUDE = -I ./App/Include/
CXXFLAGS = -std=c++98 -g $(INCLUDE)

PATH_SRCS = ./App/Src/
# PATH_ACPT = $(PATH_SRCS)Accept/
PATH_BOOST = $(PATH_SRCS)Boost/
PATH_CONF = $(PATH_SRCS)Config/
# PATH_LIST = $(PATH_SRCS)Listen/
# PATH_POLL = $(PATH_SRCS)Poll/
PATH_REQ = $(PATH_SRCS)Request/
PATH_RES = $(PATH_SRCS)Response/
PATH_SERV = $(PATH_SRCS)Server/
PATH_SOCK = $(PATH_SRCS)Socket/

PATH_OBJS = ./Objs/

# SRCS =	$(PATH_ACPT)Accept.cpp \
# 		$(PATH_BIND)Bind.cpp \
# 		$(PATH_CONF)Config.cpp \
# 		$(PATH_LIST)Listen.cpp \
# 		$(PATH_POLL)Poll.cpp \
# 		$(PATH_SERV)Server.cpp \
#		$(PATH_SOCK)Socket.cpp \
# 		$(PATH_SRCS)main.cpp
	
SRCS =	$(PATH_SRCS)main.cpp \
		$(PATH_SERV)Server.cpp \
		$(PATH_CONF)Config.cpp \
		$(PATH_BOOST)String.cpp \
		$(PATH_REQ)Request.cpp \
		$(PATH_RES)Response.cpp \
		$(PATH_SOCK)Socket.cpp 


OBJS += $(patsubst $(PATH_SRCS)%.cpp, $(PATH_OBJS)%.o, $(SRCS))

all:	${NAME} 

${NAME}:	${OBJS}
	$(CC) $(CXXFLAGS) -o $(NAME) $(OBJS)

$(PATH_OBJS)%.o: $(PATH_SRCS)%.cpp
	@mkdir -p $(PATH_OBJS)
# @mkdir -p $(PATH_OBJS)Accept/
	@mkdir -p $(PATH_OBJS)Boost/
	@mkdir -p $(PATH_OBJS)Config/
# @mkdir -p $(PATH_OBJS)Listen/
# @mkdir -p $(PATH_OBJS)Poll/
	@mkdir -p $(PATH_OBJS)Request/
	@mkdir -p $(PATH_OBJS)Response/
	@mkdir -p $(PATH_OBJS)Server/
	@mkdir -p $(PATH_OBJS)Socket/
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:		
	rm -rf ${OBJS}

fclean:		clean
	rm -rf ${NAME}

re:			fclean
	clear
	make all

.PHONY: all $(PATH_OBJS) clean fclean re