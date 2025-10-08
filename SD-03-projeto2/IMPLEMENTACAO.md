# Guia de Implementação - Projeto 2 SD

## Visão Geral da Arquitetura

O projeto implementa uma arquitetura cliente-servidor usando TCP sockets e Protocol Buffers:

```
┌─────────────┐                         ┌─────────────┐
│   CLIENTE   │                         │  SERVIDOR   │
├─────────────┤                         ├─────────────┤
│ list_client │ ◄──── Interface ────►  │ list_server │
├─────────────┤       do utilizador    ├─────────────┤
│ client_stub │ ◄── Camada RPC ────►   │  list_skel  │
├─────────────┤    (conversão de       ├─────────────┤
│network_client│◄─ chamadas locais ──► │network_server│
└─────────────┘    em mensagens)       └─────────────┘
       │                                      │
       └────────── TCP Socket ────────────────┘
              (Protocol Buffers)
```

---

## 1. Protocol Buffers - Definição de Mensagens

### Ficheiro: `sdmessage.proto`

Define a estrutura das mensagens trocadas entre cliente e servidor:

**Estruturas principais:**
- `Data`: Representa um carro (ano, preço, marca, modelo, combustível)
- `message_t`: Envelope de comunicação com opcode, tipo de conteúdo e dados

**Opcodes (operações):**
- `OP_ADD` (10): Adicionar carro
- `OP_GET` (20): Obter carro(s)
- `OP_DEL` (30): Remover carro
- `OP_SIZE` (40): Tamanho da lista
- `OP_GETMODELS` (50): Lista de modelos
- `OP_ORDER` (70): Ordenar por ano

**Content Types (tipo de dados):**
- `CT_DATA`: Dados completos de um carro
- `CT_MARCA`, `CT_YEAR`, `CT_MODEL`: Critérios de pesquisa
- `CT_RESULT`: Resultado numérico
- `CT_LIST`: Array de carros ou strings

---

## 2. Módulos Base (Projeto 1)

### `data.c/h` - Estrutura de Dados do Carro

**Funções principais:**
- `data_create()`: Aloca e inicializa um carro
- `data_destroy()`: Liberta memória de um carro
- `data_dup()`: Duplica um carro
- `data_replace()`: Substitui dados de um carro

**Gestão de memória:**
- `modelo` é alocado dinamicamente com `strdup()`
- Sempre libertar com `free()` antes de destruir

### `list.c/h` - Lista Encadeada

**Estrutura:**
```c
struct car_t {
    struct data_t *data;  // Dados do carro
    struct car_t *next;   // Próximo nó
};

struct list_t {
    int size;             // Número de carros
    struct car_t *head;   // Cabeça da lista
};
```

**Funções principais:**
- `list_create()`: Cria lista vazia
- `list_add()`: Adiciona no final (duplica dados)
- `list_remove_by_model()`: Remove por modelo
- `list_get_by_marca()`: Busca por marca (retorna cópia)
- `list_get_by_year()`: Busca por ano (retorna array de cópias)
- `list_order_by_year()`: Ordena in-place (bubble sort)
- `list_get_all()`: Retorna array de ponteiros (NÃO duplica)

**Importante:**
- `list_add()` **duplica** os dados (ownership da lista)
- `list_get_*()` retornam **cópias** (caller deve destruir)
- `list_get_all()` retorna **ponteiros internos** (não destruir!)

---

## 3. Funções Auxiliares de Rede

### `message-private.c/h`

**`write_all(int sock, void *buf, int len)`**
- Garante que TODOS os bytes são escritos
- Lida com writes parciais e EINTR
- Retorna 0 (sucesso) ou -1 (erro)

**`read_all(int sock, void *buf, int len)`**
- Garante que TODOS os bytes são lidos
- Lida com reads parciais e EINTR
- Retorna 0 (sucesso) ou -1 (erro)

**Por que são necessárias?**
- `write()` e `read()` podem retornar menos bytes que o solicitado
- Sinais (EINTR) podem interromper as operações
- Necessário em rede para garantir integridade das mensagens

---

## 4. Cliente

### 4.1. `network_client.c` - Comunicação TCP

**`network_connect(struct rlist_t *rlist)`**
1. Resolve hostname (via `gethostbyname` ou `inet_pton`)
2. Cria socket TCP (`socket()`)
3. Conecta ao servidor (`connect()`)
4. Guarda socket descriptor em `rlist->sockfd`

**`network_send_receive(struct rlist_t *rlist, MessageT *msg)`**
1. Serializa mensagem: `message_t__pack()`
2. Envia tamanho (4 bytes, network byte order): `htonl()`
3. Envia mensagem serializada: `write_all()`
4. Recebe tamanho da resposta: `read_all()` + `ntohl()`
5. Recebe resposta serializada: `read_all()`
6. Desserializa: `message_t__unpack()`
7. Retorna MessageT* (caller deve libertar com `message_t__free_unpacked()`)

**Formato do protocolo:**
```
┌─────────────┬─────────────────┐
│  Tamanho    │   Mensagem      │
│  (4 bytes)  │   (N bytes)     │
│  uint32_t   │   Serializada   │
│ (Big Endian)│  (Protobuf)     │
└─────────────┴─────────────────┘
```

### 4.2. `client_stub.c` - Camada RPC

**Responsabilidade:** Converter chamadas de função em mensagens protobuf.

**Exemplo: `rlist_add(struct rlist_t *rlist, struct data_t *car)`**

1. **Criar mensagem:**
   ```c
   MessageT msg = MESSAGE_T__INIT;
   msg.opcode = MESSAGE_T__OPCODE__OP_ADD;
   msg.c_type = MESSAGE_T__C_TYPE__CT_DATA;
   ```

2. **Converter data_t → Data (protobuf):**
   ```c
   Data *pb_data = malloc(sizeof(Data));
   data__init(pb_data);
   pb_data->ano = car->ano;
   pb_data->modelo = strdup(car->modelo);
   // ... outros campos
   msg.data = pb_data;
   ```

3. **Enviar e receber:**
   ```c
   MessageT *response = network_send_receive(rlist, &msg);
   ```

4. **Verificar resposta:**
   ```c
   if (response->opcode == MESSAGE_T__OPCODE__OP_ADD + 1) {
       result = response->result;
   }
   ```

5. **Libertar memória:**
   ```c
   free(pb_data->modelo);
   free(pb_data);
   message_t__free_unpacked(response, NULL);
   ```

**Padrão geral:**
- Opcode de resposta bem-sucedida = opcode pedido + 1
- Opcode de erro = `OP_ERROR`
- Sempre verificar `opcode` e `c_type` da resposta

### 4.3. `list_client.c` - Interface do Utilizador

**Loop principal:**
1. Ler comando do stdin (`fgets()`)
2. Fazer parse com `sscanf()`
3. Chamar função apropriada do `client_stub`
4. Apresentar resultado ao utilizador

**Validação de entrada:**
- Verificar número de argumentos
- Validar ranges (marca 0-4, combustível 0-3)
- Tratar erros de parsing

---

## 5. Servidor

### 5.1. `network_server.c` - Servidor TCP

**`network_server_init(short port)`**
1. Criar socket: `socket(AF_INET, SOCK_STREAM, 0)`
2. Configurar `SO_REUSEADDR` para reutilização rápida do porto
3. Bind: `bind()` ao porto especificado
4. Listen: `listen()` com backlog de 5 conexões
5. Retornar socket descriptor

**`network_main_loop(int listening_socket, struct list_t *list)`**
```c
while (!shutdown_requested) {
    // 1. Aceitar cliente
    client_socket = accept(listening_socket, ...);

    // 2. Loop de atendimento do cliente
    while (client_active) {
        // a. Receber pedido
        MessageT *request = network_receive(client_socket);

        // b. Processar no skeleton
        invoke(request, list);

        // c. Enviar resposta
        network_send(client_socket, request);

        // d. Libertar mensagem
        message_t__free_unpacked(request, NULL);
    }

    // 3. Fechar conexão do cliente
    close(client_socket);
}
```

**Importante:**
- Atende **1 cliente de cada vez** (sem threads)
- Próximo cliente só é aceite depois do anterior desconectar
- `invoke()` modifica a mensagem in-place para criar a resposta

### 5.2. `list_skel.c` - Skeleton RPC

**Responsabilidade:** Converter mensagens protobuf em operações na lista.

**`invoke(MessageT *msg, struct list_t *list)`**

Switch no opcode para decidir operação:

**Exemplo: OP_ADD**
```c
case MESSAGE_T__OPCODE__OP_ADD:
    // 1. Converter protobuf → data_t
    struct data_t *car = protobuf_to_data(msg->data);

    // 2. Executar operação
    int result = list_add(list, car);
    data_destroy(car);

    // 3. Preparar resposta (modificar msg in-place)
    msg->opcode = MESSAGE_T__OPCODE__OP_ADD + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
    msg->result = result;

    // 4. Libertar data antiga
    free(msg->data->modelo);
    free(msg->data);
    msg->data = NULL;

    return 0;
```

**Exemplo: OP_GET com CT_YEAR**
```c
case MESSAGE_T__OPCODE__OP_GET:
    if (msg->c_type == MESSAGE_T__C_TYPE__CT_YEAR) {
        // 1. Extrair ano
        int ano = msg->data->ano;

        // 2. Obter carros
        struct data_t **cars = list_get_by_year(list, ano);

        // 3. Contar e preparar array protobuf
        size_t count = 0;
        while (cars[count] != NULL) count++;

        msg->n_cars = count;
        msg->cars = malloc(sizeof(Data*) * count);

        // 4. Converter cada carro
        for (size_t i = 0; i < count; i++) {
            msg->cars[i] = data_to_protobuf(cars[i]);
            data_destroy(cars[i]);
        }
        free(cars);

        // 5. Configurar resposta
        msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_LIST;

        return 0;
    }
```

**Padrão geral:**
1. Extrair parâmetros da mensagem
2. Chamar função da lista
3. Converter resultado para protobuf
4. Modificar mensagem in-place com resposta
5. Libertar memória antiga da mensagem
6. Opcode resposta = opcode pedido + 1

### 5.3. `list_server.c` - Main do Servidor

**Fluxo:**
1. Parse dos argumentos (porto)
2. Registar handler de SIGINT (Ctrl+C)
3. Inicializar lista: `list_skel_init()`
4. Inicializar servidor TCP: `network_server_init(port)`
5. Entrar no loop principal: `network_main_loop()`
6. Cleanup ao terminar

---

## 6. Gestão de Memória

### Regras Críticas

**Protocol Buffers:**
- Mensagens recebidas: `message_t__unpack()` → libertar com `message_t__free_unpacked()`
- Strings em protobuf: Usar `strdup()` ao criar, libertar manualmente
- Arrays: Alocar com `malloc()`, libertar cada elemento + array

**Data e List:**
- `list_add()` duplica dados → lista é dona da memória
- `list_get_*()` retorna cópias → caller deve destruir
- `list_get_all()` retorna ponteiros internos → NÃO destruir data_t*

**Network:**
- Buffers de serialização: Alocar, usar, libertar imediatamente
- Mensagens: Libertar com `message_t__free_unpacked()` após uso

### Checklist Anti-Memory-Leak

Para cada `malloc()`:
- ✓ Verificar se retorno é NULL
- ✓ Ter caminho de erro que liberta tudo já alocado
- ✓ Ter `free()` correspondente em todos os caminhos

Para cada `strdup()`:
- ✓ Verificar se retorno é NULL
- ✓ Ter `free()` antes de perder ponteiro

Para cada `message_t__unpack()`:
- ✓ Ter `message_t__free_unpacked()` correspondente

---

## 7. Tratamento de Erros

### Erros de Rede

**Cliente:**
- Falha de conexão → Retornar NULL, informar utilizador
- Timeout/desconexão → Retornar NULL em `network_send_receive()`
- Mensagem inválida → Verificar opcode/c_type, retornar erro

**Servidor:**
- Cliente desconecta → Fechar socket, aceitar próximo
- Mensagem inválida → Enviar `OP_ERROR` + `CT_NONE`
- Erro interno → Logar erro, continuar a servir

### Erros de Operações

**Lista:**
- Elemento não encontrado → Retornar 1 (not found) vs -1 (erro)
- Memória insuficiente → Retornar NULL, não modificar estado
- Parâmetro inválido → Retornar -1

**Protobuf:**
- Desserialização falha → Retornar NULL
- Tamanho inválido → Rejeitar (limite 10MB)

---

## 8. Compilação e Testes

### Compilação

```bash
# 1. Instalar dependências
./install_deps.sh

# 2. Gerar código protobuf
./generate_proto.sh

# 3. Compilar tudo
make all
```

### Testes Básicos

**Terminal 1 (Servidor):**
```bash
./binary/list_server 8080
```

**Terminal 2 (Cliente):**
```bash
./binary/list_client localhost:8080
> add 2020 25000 1 BMW_M3 0
> add 2019 30000 1 BMW_M5 0
> size
> get_model_list
> get_by_year 2020
> quit
```

### Testes de Robustez

1. **Desconexão abrupta:** Ctrl+C no cliente → servidor deve continuar
2. **Comandos inválidos:** Testar parsing e validação
3. **Lista vazia:** Testar operações em lista vazia
4. **Múltiplas conexões sequenciais:** Vários clientes seguidos
5. **Memory leaks:** Usar `valgrind` (Linux) ou `leaks` (macOS)

```bash
# macOS
leaks --atExit -- ./binary/list_server 8080

# Linux
valgrind --leak-check=full ./binary/list_server 8080
```

---

## 9. Extensões Futuras (Projeto 3)

- **Multi-threading:** Múltiplos clientes simultâneos
- **Sincronização:** Mutexes para acesso concurrent à lista
- **Persistência:** Guardar lista em ficheiro
- **Replicação:** Múltiplos servidores com consenso
- **Tolerância a falhas:** Deteção e recuperação de falhas

---

## 10. Checklist de Entrega

- [ ] Todos os ficheiros `.c` implementados
- [ ] Makefile com targets: `all`, `liblist`, `list_client`, `list_server`, `clean`
- [ ] README.md com instruções de compilação e uso
- [ ] Código compila sem warnings (-Wall -Wextra)
- [ ] Cliente e servidor comunicam corretamente
- [ ] Todos os comandos funcionam
- [ ] Gestão de memória sem leaks
- [ ] Tratamento de erros robusto
- [ ] Código comentado e formatado
- [ ] Estrutura de pastas correta (include/, source/, object/, lib/, binary/)

---

**Dica final:** Testar incremental! Implementar módulo a módulo e testar cada um antes de passar ao próximo.
