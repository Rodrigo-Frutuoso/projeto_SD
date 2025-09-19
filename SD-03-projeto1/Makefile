# Grupo 03
# Rodrigo Frutuoso - 61865
# Simão Alexandre - 61874
# Tiago Leite - 61863

BIN_DIR = binary
INC_DIR = include
OBJ_DIR = object
SRC_DIR = source
TEST_DIR = tests
CC = gcc
CFLAGS = -Wall -Wextra -Werror

OBJECTS_FILES = data.o list.o serialization.o
OBJECTS = $(addprefix $(OBJ_DIR)/, $(OBJECTS_FILES))

TEST_NAMES = test_data test_list test_serialization
TEST_BINARIES = $(addprefix $(BIN_DIR)/, $(TEST_NAMES))


all: tests

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	@\rm -f $(OBJ_DIR)/* || true

fclean: clean
	@\rm -f $(BIN_DIR)/* || true

re: fclean all

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_DIR)

tests: $(TEST_BINARIES)

$(BIN_DIR)/test_%: $(TEST_DIR)/test_%.c $(OBJECTS) | $(BIN_DIR)
	@$(CC) $(CFLAGS) $< $(OBJECTS) -I $(INC_DIR) -o $@

test: tests
	@echo "Running tests..."
	@set -e; \
	for t in $(TEST_BINARIES); do \
	  echo "--- $$t"; \
	  "$$t"; \
	  echo; \
	done; \
	echo "All tests passed."

.PHONY: tests test clean fclean re all
.SECONDARY: $(OBJECTS)
