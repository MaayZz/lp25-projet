CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -std=c11 -Iinclude
LDFLAGS = -lncurses

SRC_DIR = src
OBJ_DIR = obj

SRCS = \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/manager.c \
	$(SRC_DIR)/process.c \
	$(SRC_DIR)/network.c \
	$(SRC_DIR)/ui.c \
	$(SRC_DIR)/config.c \
	$(SRC_DIR)/utils.c

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TARGET = lp25

.PHONY: all clean fclean re debug

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += -g
debug: re

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(TARGET)

re: fclean all
