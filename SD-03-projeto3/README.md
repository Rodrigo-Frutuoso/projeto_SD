# SD-03-projeto2

Grupo 03
- Rodrigo Frutuoso - 61865
- Simão Alexandre - 61874
- Tiago Leite - 61863

Projeto 2 — Sistemas Distribuídos 2025/2026

Este projeto implementa um sistema cliente-servidor de gestão de inventário para um stand automóvel, usando o paradigma RPC (Remote Procedure Calls) e Protocol Buffers para comunicação.

## Arquitetura

### Cliente (`list_client`)
- **list_client.c** - programa principal com interface de utilizador
- **client_stub.c/h** - stub RPC do lado do cliente (adaptação de chamadas)
- **network_client.c/h** - módulo de comunicação TCP do cliente

### Servidor (`list_server`)
- **list_server.c** - programa principal do servidor
- **list_skel.c/h** - skeleton RPC do lado do servidor (processamento de pedidos)
- **network_server.c/h** - módulo de comunicação TCP do servidor

### Módulos auxiliares
- **message-private.c/h** - funções `read_all()` e `write_all()` para I/O completo
- **data.c/h** e **list.c/h** - estruturas de dados do projeto 1
- **sdmessage.pb-c.c/h** - gerados automaticamente pelo Protocol Buffers

## Notas de implementação

### Protocolo de comunicação
- Usa **Protocol Buffers** (sdmessage.proto) para serialização/desserialização
- Antes de cada mensagem, envia-se um **short (2 bytes)** em network byte order com o tamanho
- Mensagens seguem formato definido: `{opcode, c_type, [dados]}`
- Resposta bem-sucedida: `opcode = pedido + 1`
- Resposta com erro: `{OP_ERROR, CT_NONE}`

### Características técnicas
- Servidor atende **um cliente de cada vez** (concorrência no projeto 3)
- Funções `read_all()` e `write_all()` garantem leitura/escrita completa em sockets
- Servidor ignora **SIGPIPE** para evitar crashes inesperados
- Usa **SO_REUSEADDR** para reinicialização rápida do servidor
- Gestão rigorosa de memória para evitar *memory leaks*

### Simplificações implementadas
- Função auxiliar `validate_response()` no cliente para validação consistente
- Função auxiliar `create_data_message()` no servidor para reduzir duplicação de código
- Comando `quit` encerra elegantemente o cliente

## Estrutura de diretórios
```
SD-03-projeto2/
├── include/          # Ficheiros .h (headers)
├── source/           # Ficheiros .c (código fonte)
├── object/           # Ficheiros .o (gerados pelo make)
├── lib/              # liblist.a (gerada pelo make)
├── binary/           # Executáveis list_client e list_server
├── dependencies/     # Dependências de compilação
├── Makefile          # Automatização da compilação
├── sdmessage.proto   # Definição Protocol Buffers
└── README.md         # Este ficheiro
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
# Terminal 1
./binary/list_server 12345

# Terminal 2
./binary/list_client 127.0.0.1:12345
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
