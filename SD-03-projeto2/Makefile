# Grupo 03
# Rodrigo Frutuoso - 61865
# Simão Alexandre - 61874
# Tiago Leite - 61863

# ============================================================================
# VARIÁVEIS DE CONFIGURAÇÃO
# ============================================================================

# Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
LDFLAGS = -lprotobuf-c

# Diretorias
INCLUDE_DIR = include
SOURCE_DIR = source
OBJECT_DIR = object
LIB_DIR = lib
BINARY_DIR = binary

# Bibliotecas
LIB_LIST = $(LIB_DIR)/liblist.a

# Ficheiros objeto da biblioteca
LIB_OBJECTS = $(OBJECT_DIR)/data.o $(OBJECT_DIR)/list.o

# Ficheiros objeto comuns (usados por cliente e servidor)
COMMON_OBJECTS = $(OBJECT_DIR)/message-private.o $(OBJECT_DIR)/sdmessage.pb-c.o

# Ficheiros objeto do servidor
SERVER_OBJECTS = $(OBJECT_DIR)/list_server.o \
                 $(OBJECT_DIR)/network_server.o \
                 $(OBJECT_DIR)/list_skel.o

# Ficheiros objeto do cliente
CLIENT_OBJECTS = $(OBJECT_DIR)/list_client.o \
                 $(OBJECT_DIR)/network_client.o \
                 $(OBJECT_DIR)/client_stub.o

# Executáveis
SERVER_EXEC = $(BINARY_DIR)/list_server
CLIENT_EXEC = $(BINARY_DIR)/list_client

# ============================================================================
# TARGETS PRINCIPAIS
# ============================================================================

# Target padrão: compila tudo
all: liblist list_server list_client

# Cria a biblioteca estática liblist.a
liblist: $(LIB_DIR) $(OBJECT_DIR) $(LIB_LIST)

# Compila o servidor
list_server: liblist $(SERVER_EXEC)

# Compila o cliente
list_client: liblist $(CLIENT_EXEC)

# ============================================================================
# CRIAÇÃO DE DIRETORIAS
# ============================================================================

$(OBJECT_DIR):
	mkdir -p $(OBJECT_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

$(BINARY_DIR):
	mkdir -p $(BINARY_DIR)

# ============================================================================
# COMPILAÇÃO DA BIBLIOTECA
# ============================================================================

$(LIB_LIST): $(LIB_OBJECTS)
	ar -rcs $@ $^

$(OBJECT_DIR)/data.o: $(SOURCE_DIR)/data.c $(INCLUDE_DIR)/data.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJECT_DIR)/list.o: $(SOURCE_DIR)/list.c $(INCLUDE_DIR)/list.h $(INCLUDE_DIR)/list-private.h $(INCLUDE_DIR)/data.h
	$(CC) $(CFLAGS) -c $< -o $@

# ============================================================================
# COMPILAÇÃO DOS FICHEIROS COMUNS
# ============================================================================

$(OBJECT_DIR)/message-private.o: $(SOURCE_DIR)/message-private.c $(INCLUDE_DIR)/message-private.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJECT_DIR)/sdmessage.pb-c.o: $(SOURCE_DIR)/sdmessage.pb-c.c $(INCLUDE_DIR)/sdmessage.pb-c.h
	$(CC) $(CFLAGS) -c $< -o $@

# ============================================================================
# COMPILAÇÃO DO SERVIDOR
# ============================================================================

$(SERVER_EXEC): $(BINARY_DIR) $(SERVER_OBJECTS) $(COMMON_OBJECTS) $(LIB_LIST)
	$(CC) $(SERVER_OBJECTS) $(COMMON_OBJECTS) $(LIB_LIST) $(LDFLAGS) -o $@

$(OBJECT_DIR)/list_server.o: $(SOURCE_DIR)/list_server.c $(INCLUDE_DIR)/network_server.h $(INCLUDE_DIR)/list_skel.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJECT_DIR)/network_server.o: $(SOURCE_DIR)/network_server.c $(INCLUDE_DIR)/network_server.h $(INCLUDE_DIR)/network_server-private.h $(INCLUDE_DIR)/sdmessage.pb-c.h $(INCLUDE_DIR)/message-private.h $(INCLUDE_DIR)/list_skel.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJECT_DIR)/list_skel.o: $(SOURCE_DIR)/list_skel.c $(INCLUDE_DIR)/list_skel.h $(INCLUDE_DIR)/list.h $(INCLUDE_DIR)/sdmessage.pb-c.h $(INCLUDE_DIR)/data.h
	$(CC) $(CFLAGS) -c $< -o $@

# ============================================================================
# COMPILAÇÃO DO CLIENTE
# ============================================================================

$(CLIENT_EXEC): $(BINARY_DIR) $(CLIENT_OBJECTS) $(COMMON_OBJECTS) $(LIB_LIST)
	$(CC) $(CLIENT_OBJECTS) $(COMMON_OBJECTS) $(LIB_LIST) $(LDFLAGS) -o $@

$(OBJECT_DIR)/list_client.o: $(SOURCE_DIR)/list_client.c $(INCLUDE_DIR)/client_stub.h $(INCLUDE_DIR)/data.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJECT_DIR)/network_client.o: $(SOURCE_DIR)/network_client.c $(INCLUDE_DIR)/network_client.h $(INCLUDE_DIR)/client_stub.h $(INCLUDE_DIR)/sdmessage.pb-c.h $(INCLUDE_DIR)/message-private.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJECT_DIR)/client_stub.o: $(SOURCE_DIR)/client_stub.c $(INCLUDE_DIR)/client_stub.h $(INCLUDE_DIR)/client_stub-private.h $(INCLUDE_DIR)/network_client.h $(INCLUDE_DIR)/data.h $(INCLUDE_DIR)/sdmessage.pb-c.h
	$(CC) $(CFLAGS) -c $< -o $@

# ============================================================================
# LIMPEZA
# ============================================================================

clean:
	rm -rf $(OBJECT_DIR) $(LIB_DIR) $(BINARY_DIR)

# ============================================================================
# PHONY TARGETS
# ============================================================================

.PHONY: all liblist list_server list_client clean
