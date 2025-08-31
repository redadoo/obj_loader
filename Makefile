# Project
NAME        := obj_loader

# Compiler & Flags
CXX         := g++
CXXFLAGS    := -std=c++17 -O3 -Wall -Wextra -Werror
LDFLAGS     := -lraylib -ldl -lm -lpthread -lGL -lrt -lX11

# Directories
SRCDIR      := src
OBJDIR      := .obj
INCDIRS     := test src 

# Source files
SRCFILES    := main.cpp
SRC         := $(addprefix $(SRCDIR)/, $(SRCFILES))
OBJ         := $(addprefix $(OBJDIR)/, $(SRCFILES:.cpp=.o))
DEPS        := $(OBJ:.o=.d)

# Include flags
INCFLAGS    := $(addprefix -I, $(INCDIRS))

# Colors
GREEN       := \033[32m
GRAY        := \033[2;37m
YELLOW      := \033[1;33m
RESET       := \033[0m

MAKEFLAGS  += --no-print-directory

.PHONY: all clean fclean re run leaks

all: $(NAME)

$(NAME): $(OBJ)
	@echo "$(GRAY)[Linking] $(NAME)$(RESET)"
	@$(CXX) $^ -o $@ $(LDFLAGS) $(INCFLAGS)
	@echo "$(GREEN)✓ Build successful$(RESET)"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "$(YELLOW)[Compiling] $<$(RESET)"
	@$(CXX) $(CXXFLAGS) $(INCFLAGS) -MMD -MP -c $< -o $@

run: $(NAME)
	@./$(NAME)

leaks: $(NAME)
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./$(NAME)

clean:
	@rm -rf $(OBJDIR)
	@echo "$(GRAY)✗ Object files cleaned$(RESET)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(GRAY)✗ Binary cleaned$(RESET)"

re: fclean all

-include $(DEPS)