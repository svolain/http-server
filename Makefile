# WEBSERV
NAME			:=	webserv

# COMPILER
CPP						:=	c++ -std=c++20
FLAGS					:=	-Wall -Wextra -Werror -Wpedantic
DFLAGS				:=	-DDEBUG -g
# COLORS
GREEN					:=	\033[32m
RED						:=	\033[41m
EC						:=	\033[0m

# SOURCE_FILES
debug: FLAGS += $(DFLAGS)

WEBSERV_NAME	:=	WebServ.cpp HttpParser.cpp Connection.cpp ClientConnection.cpp \
									CgiConnection.cpp HttpResponse.cpp VirtualHost.cpp Location.cpp \
									Socket.cpp ConfigParser.cpp

WEBSERV_PATH	:=	Webserv/
WEBSERV				:=	$(addprefix $(WEBSERV_PATH), $(WEBSERV_NAME))

SRCS					:=	main.cpp $(WEBSERV)
SRCS_PATH			:=	srcs/

# HEADER_FILES
INCLUDES_PATH	:=	includes/
INCLUDES		:=	$(addprefix -I , $(INCLUDES_PATH))

# OBJECT_FILES
OBJS_PATH		:=	objs/
OBJS			:=	$(addprefix $(OBJS_PATH), $(SRCS:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJS_PATH) $(OBJS)
	@$(CPP) $(FLAGS) $(INCLUDES) $(OBJS) -o $(NAME)
	@echo "$(GREEN)$(NAME) created successfully!$(EC)"

$(OBJS_PATH)%.o: $(SRCS_PATH)%.cpp
	@$(CPP) $(FLAGS) $(INCLUDES) -c $< -o $@

$(OBJS_PATH):
	@mkdir -p $(OBJS_PATH)
	@mkdir -p $(OBJS_PATH)$(WEBSERV_PATH) 

clean:
	@rm -rf $(OBJS_PATH)
	@echo "$(RED)Object files removed!$(EC)"

fclean: clean
	@rm -rf $(NAME)
	@echo "$(RED)Full clean up completed successfully!$(EC)"

re: fclean all

debug: fclean all

.PHONY: all clean fclean re
