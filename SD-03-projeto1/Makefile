# Grupo 03
# Rodrigo Frutuoso - 61865
# Simão Alexandre - 61874
# Tiago Leite - 61863

BIN_DIR = binary
INC_DIR = include
OBJ_DIR = object
SRC_DIR = source
CC = gcc
CFLAGS = -Wall -Wextra -Werror

OBJECTS_FILES = data.o list-private.o list.o serialization.o
OBJECTS = $(addprefix $(OBJ_DIR)/, $(OBJECTS_FILES))

all: $(BIN_DIR) $(OBJ_DIR) $(OBJECTS)
	@$(CC) $(OBJECTS) -o $(BIN_DIR)/SD_projeto1

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	@rm -f $(OBJ_DIR)/*
	@rm -f $(BIN_DIR)/SD_projeto1

re: clean all

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_DIR)
