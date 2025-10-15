# PROJETO 2 - SISTEMAS DISTRIBUÍDOS (SD 2025/2026)
# Faculdade de Ciências da Universidade de Lisboa
# Objetivo: Implementar um sistema Cliente-Servidor com sockets TCP e Protocol Buffers (protobuf-c)
# Linguagem: C
# Tema: Sistema de gestão de inventário de carros (stand automóvel)

# ------------------------------------------------------------------------------------
# DESCRIÇÃO GERAL:
# O sistema é composto por dois programas:
#   - list_server <port>  → servidor TCP que mantém uma lista ligada de carros.
#   - list_client <server>:<port> → cliente que envia comandos para o servidor.
#
# Comunicação entre cliente e servidor:
#   - Usa sockets TCP.
#   - Usa mensagens serializadas com Protocol Buffers (ficheiro sdmessage.proto).
#   - Cada pedido (opcode) enviado pelo cliente tem uma resposta (opcode + 1) se for bem-sucedido.
#   - Em caso de erro, o servidor responde com OP_ERROR e CT_NONE.
#
# ------------------------------------------------------------------------------------
# COMANDOS SUPORTADOS PELO CLIENTE:
#   add <data>
#   remove <model>
#   get_by_marca <marca>
#   get_by_year <ano>
#   get_model_list
#   get_list_ordered_by_year
#   size
#   quit
#
# ------------------------------------------------------------------------------------
# ESTRUTURA DE DIRETÓRIOS (obrigatória no ZIP final):
#
# grupoXX/
# ├── include/   → ficheiros .h
# ├── source/    → ficheiros .c
# ├── object/    → ficheiros .o
# ├── lib/       → biblioteca liblist.a
# ├── binary/    → executáveis (list_client e list_server)
# └── Makefile
#
# ------------------------------------------------------------------------------------
# FICHEIROS A IMPLEMENTAR (TU):
#
# CLIENTE:
#   - list_client.c        → programa principal (main) que lê comandos do utilizador.
#   - client_stub.c        → cria mensagens protobuf e envia-as via rede (RPC stub).
#   - network_client.c     → gere sockets TCP e serialização/deserialização.
#
# SERVIDOR:
#   - list_server.c        → programa principal (main) do servidor.
#   - list_skel.c          → interpreta pedidos e executa operações na lista.
#   - network_server.c     → gere sockets TCP e comunicação (receber/enviar mensagens).
#
# OPCIONAL (RECOMENDADO):
#   - message-private.c / message-private.h → funções utilitárias read_all() e write_all().
#
# ------------------------------------------------------------------------------------
# FICHEIROS FORNECIDOS (NÃO ALTERAR):
#   - sdmessage.proto → descrição das mensagens protobuf.
#   - sdmessage.pb-c.c / sdmessage.pb-c.h → gerados pelo protoc.
#   - data.c/h e list.c/h → do Projeto 1 (podes usar os teus ou os fornecidos).
#   - client_stub.h, network_client.h, network_server.h, list_skel.h → interfaces fixas.
#
# ------------------------------------------------------------------------------------
# FUNÇÕES QUE DEVEM EXISTIR:
#
# CLIENTE:
#   struct rlist_t *rlist_connect(char *address_port);
#   int rlist_disconnect(struct rlist_t *rlist);
#   int rlist_add(struct rlist_t *rlist, struct data_t *car);
#   int rlist_remove_by_model(struct rlist_t *rlist, const char *modelo);
#   struct data_t *rlist_get_by_marca(struct rlist_t *rlist, enum marca_t marca);
#   struct data_t **rlist_get_by_year(struct rlist_t *rlist, int ano);
#   int rlist_order_by_year(struct rlist_t *rlist);
#   int rlist_size(struct rlist_t *rlist);
#   char **rlist_get_model_list(struct rlist_t *rlist);
#   int rlist_free_model_list(char **models);
#
#   // Network (cliente)
#   int network_connect(struct rlist_t *rlist);
#   MessageT *network_send_receive(struct rlist_t *rlist, MessageT *msg);
#   int network_close(struct rlist_t *rlist);
#
# SERVIDOR:
#   int network_server_init(short port);
#   int network_main_loop(int listening_socket, struct list_t *list);
#   MessageT *network_receive(int client_socket);
#   int network_send(int client_socket, MessageT *msg);
#   int network_server_close(int socket);
#   void network_server_request_shutdown(void);
#
#   // Skeleton (servidor)
#   struct list_t *list_skel_init();
#   int list_skel_destroy(struct list_t *list);
#   int invoke(MessageT *msg, struct list_t *list);
#
# ------------------------------------------------------------------------------------
# DICAS:
# - Implementar read_all() e write_all() para garantir que os buffers são completamente enviados/recebidos.
# - Ignorar SIGPIPE com signal(SIGPIPE, SIG_IGN) para evitar crashes.
# - Usar setsockopt(..., SO_REUSEADDR, ...) no servidor para reiniciar facilmente.
# - Libertar sempre memória alocada (valgrind deve reportar 0 leaks).
# - O servidor deve atender apenas um cliente de cada vez (sem threads nesta fase).
#
# ------------------------------------------------------------------------------------
# MAKEFILE (obrigatório):
#
# Targets obrigatórios:
#   all          → compila tudo.
#   liblist      → cria biblioteca liblist.a com data.o e list.o.
#   list_client  → compila o cliente.
#   list_server  → compila o servidor.
#   clean        → apaga ficheiros .o, .a e binários.
#
# ------------------------------------------------------------------------------------
# REGRAS DE ENTREGA:
# - Submeter um ZIP: grupoXX-projeto2.zip
# - Prazo: 26/10/2025 às 23:59
# - ZIP deve conter: README, Makefile e as pastas indicadas.
# - Compilação testada em Linux dos laboratórios da FCUL.
# ------------------------------------------------------------------------------------
# AGORA GERA O CÓDIGO C PARA ESTES FICHEIROS UM A UM.
# Segue sempre o estilo de programação modular, usando boas práticas de memória e rede.
# Evita alterar cabeçalhos fornecidos (.h fixos).
# Começa por criar o código base para cada ficheiro com as assinaturas e estruturas principais.
