# Makefile para Projeto 2 - Sistemas Distribuídos
# FCUL 2025/2026

CC = gcc
CFLAGS = -Wall -Wextra -g -I./include -I/opt/homebrew/opt/protobuf-c/include
LDFLAGS = -L./lib -L/opt/homebrew/opt/protobuf-c/lib
LIBS = -lprotobuf-c

# Diretórias
SRC_DIR = source
OBJ_DIR = object
INC_DIR = include
LIB_DIR = lib
BIN_DIR = binary

# Ficheiros objeto para liblist.a (módulos do Projeto 1)
LIBLIST_OBJS = $(OBJ_DIR)/data.o $(OBJ_DIR)/list.o

# Ficheiros objeto do cliente
CLIENT_OBJS = $(OBJ_DIR)/list_client.o \
              $(OBJ_DIR)/client_stub.o \
              $(OBJ_DIR)/network_client.o \
              $(OBJ_DIR)/message-private.o \
              $(OBJ_DIR)/sdmessage.pb-c.o

# Ficheiros objeto do servidor
SERVER_OBJS = $(OBJ_DIR)/list_server.o \
              $(OBJ_DIR)/list_skel.o \
              $(OBJ_DIR)/network_server.o \
              $(OBJ_DIR)/message-private.o \
              $(OBJ_DIR)/sdmessage.pb-c.o

# Targets principais
.PHONY: all clean liblist list_client list_server

all: liblist list_client list_server

# Criar diretórios necessários
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Biblioteca liblist.a (módulos data e list)
liblist: $(LIB_DIR)/liblist.a

$(LIB_DIR)/liblist.a: $(LIBLIST_OBJS) | $(LIB_DIR)
	ar rcs $@ $(LIBLIST_OBJS)
	@echo "Biblioteca liblist.a criada."

# Cliente
list_client: $(BIN_DIR)/list_client

$(BIN_DIR)/list_client: $(CLIENT_OBJS) $(LIB_DIR)/liblist.a | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $(CLIENT_OBJS) -llist $(LIBS)
	@echo "Cliente list_client criado."

# Servidor
list_server: $(BIN_DIR)/list_server

$(BIN_DIR)/list_server: $(SERVER_OBJS) $(LIB_DIR)/liblist.a | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $(SERVER_OBJS) -llist $(LIBS)
	@echo "Servidor list_server criado."

# Compilação dos ficheiros objeto
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Dependências específicas (headers)
$(OBJ_DIR)/data.o: $(INC_DIR)/data.h
$(OBJ_DIR)/list.o: $(INC_DIR)/list.h $(INC_DIR)/list-private.h $(INC_DIR)/data.h
$(OBJ_DIR)/message-private.o: $(INC_DIR)/message-private.h
$(OBJ_DIR)/sdmessage.pb-c.o: $(INC_DIR)/sdmessage.pb-c.h

$(OBJ_DIR)/client_stub.o: $(INC_DIR)/client_stub.h $(INC_DIR)/client_stub-private.h \
                           $(INC_DIR)/network_client.h $(INC_DIR)/data.h \
                           $(INC_DIR)/sdmessage.pb-c.h

$(OBJ_DIR)/network_client.o: $(INC_DIR)/network_client.h $(INC_DIR)/client_stub-private.h \
                              $(INC_DIR)/message-private.h $(INC_DIR)/sdmessage.pb-c.h

$(OBJ_DIR)/list_client.o: $(INC_DIR)/client_stub.h $(INC_DIR)/data.h

$(OBJ_DIR)/list_skel.o: $(INC_DIR)/list_skel.h $(INC_DIR)/list.h $(INC_DIR)/data.h \
                        $(INC_DIR)/sdmessage.pb-c.h

$(OBJ_DIR)/network_server.o: $(INC_DIR)/network_server.h $(INC_DIR)/list_skel.h \
                              $(INC_DIR)/message-private.h $(INC_DIR)/sdmessage.pb-c.h

$(OBJ_DIR)/list_server.o: $(INC_DIR)/network_server.h $(INC_DIR)/list_skel.h

# Limpeza
clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)
	@echo "Limpeza concluída."
