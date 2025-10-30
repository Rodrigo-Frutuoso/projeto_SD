# Grupo 03
# Rodrigo Frutuoso - 61865
# Simão Alexandre - 61874
# Tiago Leite - 61863

CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
LDFLAGS = -lprotobuf-c -lpthread

SRCDIR = source
OBJDIR = object
LIBDIR = lib
BINDIR = binary

PROVIDED_OBJS = $(OBJDIR)/data.o $(OBJDIR)/list.o

LIB = $(LIBDIR)/liblist.a
LIB_OBJS = $(PROVIDED_OBJS)
COMMON_OBJS = $(OBJDIR)/message-private.o $(OBJDIR)/sdmessage.pb-c.o
SERVER_OBJS = $(OBJDIR)/list_server.o $(OBJDIR)/network_server.o $(OBJDIR)/list_skel.o
CLIENT_OBJS = $(OBJDIR)/list_client.o $(OBJDIR)/network_client.o $(OBJDIR)/client_stub.o

GENERATED_OBJS = $(COMMON_OBJS) $(SERVER_OBJS) $(CLIENT_OBJS)

.SILENT:

all: liblist list_client list_server
	@echo "✅ Compilação concluída com sucesso!"

liblist: $(LIB)
	@echo "📚 Biblioteca liblist.a criada"

list_server: $(BINDIR)/list_server
	@echo "🖥️  Servidor list_server compilado"

list_client: $(BINDIR)/list_client
	@echo "💻 Cliente list_client compilado"

$(LIB): $(LIB_OBJS) | $(LIBDIR)
	@ar -rcs $@ $^

$(BINDIR)/list_server: $(SERVER_OBJS) $(COMMON_OBJS) $(LIB) | $(BINDIR)
	@$(CC) $^ $(LDFLAGS) -o $@

$(BINDIR)/list_client: $(CLIENT_OBJS) $(COMMON_OBJS) $(LIB) | $(BINDIR)
	@$(CC) $^ $(LDFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR) $(LIBDIR) $(BINDIR):
	@mkdir -p $@

clean:
	@rm -f $(GENERATED_OBJS)
	@rm -f $(BINDIR)/list_server $(BINDIR)/list_client
	@rm -rf $(LIBDIR)
	@echo "🧹 Limpeza concluída!"

.PHONY: all liblist list_server list_client clean
