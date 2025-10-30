# SD-03-projeto3

Grupo 03
- Rodrigo Frutuoso - 61865
- Simão Alexandre - 61874
- Tiago Leite - 61863

Projeto 3 — Sistemas Distribuídos 2025/2026 - **Fase 3: Servidor Multi-threaded**

Este projeto implementa um sistema cliente-servidor de gestão de inventário para um stand automóvel com suporte para **múltiplos clientes simultâneos** usando threads POSIX.

## ✨ NOVIDADES DA FASE 3

### 🚀 Servidor Multi-threaded (modelo one-thread-per-client)
- Aceita até **5 clientes simultaneamente**
- Cada cliente é atendido por uma **thread dedicada**
- Limite de clientes ativos: novas conexões são **rejeitadas com OP_BUSY**
- Clientes recebem mensagem **OP_READY** quando servidor pode atendê-los

### 📝 Sistema de Logging Completo (`server.log`)
- Todos os eventos são registados com **timestamps precisos** (gettimeofday)
- Formato: `<timestamp> <IP:port> <event_type> [<operation>] [<content_type>] [<argument>]`
- Tipos de eventos:
  - **CONNECT**: cliente estabelece ligação
  - **REQUEST**: cliente envia pedido (com detalhes completos)
  - **CLOSE**: cliente fecha ligação

### 🔒 Gestão de Concorrência com Mutexes
- **log_mutex**: protege escritas no ficheiro de log
- **clients_mutex**: protege contador de clientes ativos
- **list_mutex**: protege acesso à lista encadeada
- Sem condições de corrida nem deadlocks

### 🆕 Novos Códigos de Operação
- **OP_BUSY = 100**: servidor cheio (5 clientes ativos)
- **OP_READY = 101**: servidor disponível

### 💻 Cliente Inteligente
- Verifica estado do servidor após conexão
- Mostra **"Server busy. Try again later."** quando servidor está cheio
- Continua a funcionar normalmente quando servidor aceita conexão

## Arquitetura

### Cliente (`list_client`)
- **list_client.c** - programa principal com interface de utilizador
- **client_stub.c/h** - stub RPC do lado do cliente (adaptação de chamadas)
- **network_client.c/h** - módulo de comunicação TCP do cliente (**MODIFICADO Fase 3**)

### Servidor (`list_server`)
- **list_server.c** - programa principal do servidor
- **list_skel.c/h** - skeleton RPC do lado do servidor (processamento de pedidos)
- **network_server.c/h** - módulo de comunicação TCP multi-threaded (**MODIFICADO Fase 3**)

### Módulos auxiliares
- **message-private.c/h** - funções `read_all()` e `write_all()` para I/O completo
- **data.c/h** e **list.c/h** - estruturas de dados do projeto 1
- **sdmessage.pb-c.c/h** - gerados automaticamente pelo Protocol Buffers (**OP_BUSY/OP_READY adicionados**)

## Notas de implementação

### Protocolo de comunicação
- Usa **Protocol Buffers** (sdmessage.proto) para serialização/desserialização
- Antes de cada mensagem, envia-se um **short (2 bytes)** em network byte order com o tamanho
- Mensagens seguem formato definido: `{opcode, c_type, [dados]}`
- Resposta bem-sucedida: `opcode = pedido + 1`
- Resposta com erro: `{OP_ERROR, CT_NONE}`
- **[FASE 3]** Verificação de estado: servidor envia `OP_READY` ou `OP_BUSY` após accept()

### Características técnicas
- **[FASE 3]** Servidor atende **até 5 clientes simultaneamente** com threads POSIX
- **[FASE 3]** Modelo **one-thread-per-client**: cada cliente tem thread dedicada
- Funções `read_all()` e `write_all()` garantem leitura/escrita completa em sockets
- Servidor ignora **SIGPIPE** para evitar crashes inesperados
- Usa **SO_REUSEADDR** para reinicialização rápida do servidor
- Gestão rigorosa de memória para evitar *memory leaks*
- **[FASE 3]** Sincronização com mutexes para proteger recursos partilhados

### Thread Principal (Fase 3)
1. Aceita nova conexão com `accept()`
2. Verifica se `num_clientes_ativos < 5`
3. Se disponível: envia `OP_READY`, cria thread, regista CONNECT
4. Se cheio: envia `OP_BUSY`, fecha conexão

### Threads Secundárias (Fase 3)
1. Recebem pedidos do cliente em loop
2. Registam REQUEST no log com timestamp
3. Protegem acesso à lista com mutex
4. Processam pedido e enviam resposta
5. Ao terminar: registam CLOSE, decrementam contador, terminam thread

### Simplificações implementadas
- Função auxiliar `validate_response()` no cliente para validação consistente
- Função auxiliar `create_data_message()` no servidor para reduzir duplicação de código
- Comando `quit` encerra elegantemente o cliente
- **[FASE 3]** Funções de logging centralizadas para garantir formato consistente
- **[FASE 3]** Threads detached para libertar recursos automaticamente

## 🧪 Testes e Validação (Fase 3)

✅ Testado com Valgrind (sem memory leaks)
✅ Testado com Helgrind (sem race conditions)
✅ Servidor nunca crasha com múltiplos clientes
✅ Logs gerados corretamente com timestamps
✅ Limite de 5 clientes funciona corretamente
✅ Sincronização de threads sem deadlocks
✅ Cliente recebe mensagem "Server busy" apropriadamente

## ⚠️ Limitações Conhecidas

Nenhuma limitação conhecida. Todas as funcionalidades pedidas no enunciado da Fase 3 foram implementadas com sucesso.

## Estrutura de diretórios
```
SD-03-projeto3/
├── include/          # Ficheiros .h (headers)
├── source/           # Ficheiros .c (código fonte)
├── object/           # Ficheiros .o (gerados pelo make)
├── lib/              # liblist.a (gerada pelo make)
├── binary/           # Executáveis list_client e list_server
├── Makefile          # Automatização da compilação (com -lpthread)
├── sdmessage.proto   # Definição Protocol Buffers (com OP_BUSY/OP_READY)
├── server.log        # Log do servidor (gerado automaticamente - FASE 3)
└── README.md         # Este ficheiro
```

## 📊 Análise dos Logs (Fase 3)

O ficheiro `server.log` pode ser analisado com comandos bash:

### Número de clientes que se conectaram
```bash
cat server.log | grep CONNECT | wc -l
```

### Número de clientes que encerraram ligação
```bash
cat server.log | grep CLOSE | wc -l
```

### Número de ligações ativas
```bash
c1=`cat server.log | grep CONNECT | wc -l`; c2=`cat server.log | grep CLOSE | wc -l`; let "c=c1-c2"; echo $c
```

### Número total de pedidos processados
```bash
cat server.log | grep REQUEST | wc -l
```

### Lista de clientes únicos
```bash
cat server.log | awk '{print $2}' | sort | uniq
```

### Número de pedidos por cliente
```bash
for c in `cat server.log | awk '{print $2}' | sort | uniq`; do n=`cat server.log | grep $c | grep REQUEST | wc -l`; echo $c $n; done
```

### Tempo de conexão de cada cliente (em segundos)
```bash
for c in `cat server.log | awk '{print $2}' | sort | uniq`; do t1=`cat server.log | grep $c | grep CONNECT | awk '{print $1}'`; t2=`cat server.log | grep $c | grep CLOSE | awk '{print $1}'`; let "t=t2-t1"; echo $c $t; done
```

### Exemplo de saída do server.log
```
1761053956 127.0.0.1:7654 CONNECT
1761053972 127.0.0.1:7654 REQUEST OP_ADD CT_DATA Toyota Corolla 2021
1761053983 127.0.0.1:7654 REQUEST OP_GET CT_MODEL Corolla
1761053995 127.0.0.1:7668 CONNECT
1761054012 127.0.0.1:7654 REQUEST OP_SIZE CT_NONE
1761054015 127.0.0.1:7654 CLOSE
1761054021 127.0.0.1:7668 REQUEST OP_SIZE CT_NONE
1761054026 127.0.0.1:7668 CLOSE
```

## Geração dos ficheiros Protocol Buffers

Os ficheiros `sdmessage.pb-c.h` e `sdmessage.pb-c.c` foram gerados a partir de `sdmessage.proto` usando o compilador Protocol Buffers:

```bash
protoc-c --c_out=. sdmessage.proto
```

Após geração, os ficheiros foram movidos para:
- `sdmessage.pb-c.h` → `include/`
- `sdmessage.pb-c.c` → `source/`

**Nota:** Os ficheiros já estão incluídos e não precisam ser regenerados.

## Como compilar e executar

### Compilação
```bash
make
```

O Makefile inclui os seguintes targets:
- `make all` - compila tudo (liblist, cliente e servidor)
- `make liblist` - cria biblioteca liblist.a
- `make list_client` - compila apenas o cliente
- `make list_server` - compila apenas o servidor
- `make clean` - remove ficheiros gerados

### Execução

**Servidor:**
```bash
./binary/list_server <porta>
```

**Cliente (noutro terminal):**
```bash
./binary/list_client <servidor>:<porta>
```

**Exemplo:**
```bash
# Terminal 1 - Servidor
./binary/list_server 12345

# Terminal 2 - Cliente 1
./binary/list_client 127.0.0.1:12345

# Terminal 3 - Cliente 2
./binary/list_client 127.0.0.1:12345

# ... até 5 clientes simultâneos

# Terminal 7 - Cliente 6 (será rejeitado)
./binary/list_client 127.0.0.1:12345
# Output: Server busy. Try again later.
```

**Nota:** Use `127.0.0.1` em vez de `localhost` para evitar problemas de resolução DNS.

## Comandos disponíveis no cliente

| Comando                       | Descrição                     | Sintaxe                                                    |
|-------------------------------|-------------------------------|------------------------------------------------------------|
| `add`                         | Adiciona novo carro à lista   | `add <modelo> <ano> <preco> <marca:0-4> <combustivel:0-3>` |
| `remove`                      | Remove carro pelo modelo      | `remove <modelo>`                                          |
| `get_by_marca`                | Obtém primeiro carro da marca | `get_by_marca <marca:0-4>`                                 |
| `get_by_year`                 | Obtém carros de um ano        | `get_by_year <ano>`                                        |
| `get_list_ordered_by_year`    | Lista ordenada por ano        | `get_list_ordered_by_year`                                 |
| `get_model_list`              | Lista todos os modelos        | `get_model_list`                                           |
| `size`                        | Número de carros na lista     | `size`                                                     |
| `help`                        | Mostra ajuda                  | `help`                                                     |
| `quit`                        | Termina o cliente             | `quit`                                                     |

### Enumerações

**Marcas (0-4):**
- 0 = TOYOTA
- 1 = BMW
- 2 = RENAULT
- 3 = AUDI
- 4 = MERCEDES

**Combustíveis (0-3):**
- 0 = GASOLINA
- 1 = GASÓLEO
- 2 = ELÉTRICO
- 3 = HÍBRIDO

### Exemplos de uso
```
add Corolla 2020 25000.50 0 0
get_by_marca 0
get_by_year 2020
remove Corolla
size
quit
```

## Dependências
```bash
sudo apt-get install libprotobuf-c-dev protobuf-c-compiler
```
