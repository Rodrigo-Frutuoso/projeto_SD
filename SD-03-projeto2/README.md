# Projeto 2 - Sistema Cliente-Servidor para Gestão de Lista de Carros

**Disciplina:** Sistemas Distribuídos 2025/2026
**Instituição:** FCUL
**Grupo:** SD-03

## Descrição

Este projeto implementa um sistema cliente-servidor em C para gerir uma lista de carros. A comunicação é feita via TCP sockets e utiliza Protocol Buffers para serialização de mensagens.

## Arquitetura

### Cliente
- **list_client.c**: Interface interativa com o utilizador
- **client_stub.c**: Camada de abstração RPC (traduz chamadas locais em mensagens remotas)
- **network_client.c**: Gestão de conexão TCP e serialização/deserialização

### Servidor
- **list_server.c**: Loop principal do servidor
- **list_skel.c**: Skeleton RPC (traduz mensagens em operações na lista)
- **network_server.c**: Servidor TCP, aceita conexões e processa pedidos

### Módulos Comuns
- **data.c/h**: Estrutura de dados de um carro
- **list.c/h**: Lista encadeada de carros
- **message-private.c/h**: Funções auxiliares para I/O completo em sockets
- **sdmessage.pb-c.c/h**: Código gerado pelo Protocol Buffers

## Compilação

### Pré-requisitos
- GCC (compilador C)
- Protocol Buffers C (libprotobuf-c)
- protoc-c (compilador protobuf para C)

### Instalar dependências (macOS com Homebrew)
```bash
brew install protobuf-c
```

### Gerar código Protocol Buffers
```bash
protoc-c --c_out=./source ./sdmessage.proto
mv source/sdmessage.pb-c.c source/sdmessage.pb-c.c
mv source/sdmessage.pb-c.h include/sdmessage.pb-c.h
```

### Compilar tudo
```bash
make all
```

Isto cria:
- `lib/liblist.a`: Biblioteca com os módulos data e list
- `binary/list_client`: Executável do cliente
- `binary/list_server`: Executável do servidor

### Targets individuais
```bash
make liblist        # Apenas a biblioteca
make list_client    # Apenas o cliente
make list_server    # Apenas o servidor
make clean          # Limpar ficheiros compilados
```

## Utilização

### Iniciar o servidor
```bash
./binary/list_server <porto>
```

Exemplo:
```bash
./binary/list_server 8080
```

### Iniciar o cliente
```bash
./binary/list_client <servidor>:<porto>
```

Exemplo:
```bash
./binary/list_client localhost:8080
```

## Comandos do Cliente

- **add \<ano\> \<preco\> \<marca\> \<modelo\> \<combustivel\>**
  Adiciona um carro à lista.
  - Marca: 0=Toyota, 1=BMW, 2=Renault, 3=Audi, 4=Mercedes
  - Combustível: 0=Gasolina, 1=Gasóleo, 2=Elétrico, 3=Híbrido

  Exemplo: `add 2020 25000.50 1 M3 0`

- **remove \<modelo\>**
  Remove o primeiro carro com o modelo especificado.

  Exemplo: `remove M3`

- **get_by_marca \<marca\>**
  Obtém o primeiro carro da marca especificada.

  Exemplo: `get_by_marca 1`

- **get_by_year \<ano\>**
  Obtém todos os carros do ano especificado.

  Exemplo: `get_by_year 2020`

- **get_model_list**
  Lista todos os modelos de carros na lista.

- **size**
  Mostra o número de carros na lista.

- **help**
  Mostra a ajuda dos comandos.

- **quit**
  Termina o cliente.

## Protocolo de Comunicação

### Formato de Mensagem
1. **Tamanho** (4 bytes, network byte order): Tamanho da mensagem serializada
2. **Mensagem** (N bytes): Mensagem Protocol Buffers serializada

### Opcodes
- `OP_ADD` (10): Adicionar carro
- `OP_GET` (20): Obter carro(s)
- `OP_DEL` (30): Remover carro
- `OP_SIZE` (40): Obter tamanho
- `OP_GETMODELS` (50): Obter lista de modelos
- `OP_GETLISTBYTEAR` (60): Obter lista ordenada por ano
- `OP_ORDER` (70): Ordenar lista
- `OP_ERROR` (99): Erro

### Content Types
- `CT_DATA`: Dados de um carro
- `CT_MARCA`: Marca
- `CT_YEAR`: Ano
- `CT_MODEL`: Modelo
- `CT_RESULT`: Resultado (inteiro)
- `CT_LIST`: Lista de carros ou modelos
- `CT_NONE`: Sem conteúdo

### Regras de Resposta
- Resposta bem-sucedida: `opcode_resposta = opcode_pedido + 1`
- Resposta de erro: `opcode = OP_ERROR`, `c_type = CT_NONE`

## Estrutura de Ficheiros

```
SD-03-projeto2/
├── Makefile
├── README.md
├── sdmessage.proto
├── include/
│   ├── client_stub.h
│   ├── client_stub-private.h
│   ├── data.h
│   ├── list.h
│   ├── list-private.h
│   ├── list_skel.h
│   ├── message-private.h
│   ├── network_client.h
│   ├── network_server.h
│   ├── network_server-private.h
│   └── sdmessage.pb-c.h (gerado)
├── source/
│   ├── client_stub.c
│   ├── data.c
│   ├── list.c
│   ├── list_client.c
│   ├── list_server.c
│   ├── list_skel.c
│   ├── message-private.c
│   ├── network_client.c
│   ├── network_server.c
│   └── sdmessage.pb-c.c (gerado)
├── object/ (criado na compilação)
├── lib/ (criado na compilação)
└── binary/ (criado na compilação)
```

## Notas Importantes

1. **Gestão de Memória**: Todos os módulos fazem gestão cuidadosa de memória para evitar leaks.

2. **Fiabilidade**:
   - Funções `read_all()` e `write_all()` garantem leitura/escrita completa
   - Tratamento de erros em todas as operações de rede

3. **Limitações Atuais**:
   - Apenas 1 cliente atendido de cada vez (sem threads)
   - Tamanho máximo de mensagem: 10MB

4. **Extensões Futuras** (Projeto 3):
   - Multi-threading para múltiplos clientes simultâneos
   - Replicação e tolerância a falhas

## Autores

Grupo SD-03

## Data de Entrega

26 de outubro de 2025
