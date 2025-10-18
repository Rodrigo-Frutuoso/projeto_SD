# SD-03-projeto2

Grupo 03
- Rodrigo Frutuoso - 61865
- Simão Alexandre - 61874
- Tiago Leite - 61863

Projeto 2 — Sistemas Distribuídos 2025/2026

## Descrição
Sistema cliente-servidor para gestão de inventário de carros usando Protocol Buffers para serialização de dados.

## Compilação
Para compilar o projeto, execute:
```bash
make all
```

Este comando irá:
1. Criar a biblioteca `liblist.a` com os módulos `data.o` e `list.o`
2. Compilar o servidor `list_server`
3. Compilar o cliente `list_client`

### Targets disponíveis
- `make all`: Compila tudo (biblioteca, servidor e cliente)
- `make liblist`: Compila apenas a biblioteca
- `make list_server`: Compila apenas o servidor
- `make list_client`: Compila apenas o cliente
- `make clean`: Remove todos os ficheiros gerados

## Execução

### Servidor
```bash
./binary/list_server <porta>
```
Exemplo:
```bash
./binary/list_server 12345
```

### Cliente
```bash
./binary/list_client <servidor>:<porta>
```
Exemplo:
```bash
./binary/list_client localhost:12345
```

## Comandos do Cliente
O cliente suporta os seguintes comandos:

- `add <ano> <preco> <marca> <modelo> <combustivel>` - Adiciona um carro
- `remove <modelo>` - Remove um carro pelo modelo
- `get_by_marca <marca>` - Obtém um carro pela marca
- `get_by_year <ano>` - Obtém carros de um ano específico
- `get_list_ordered_by_year` - Obtém lista ordenada por ano
- `get_model_list` - Obtém lista de modelos
- `size` - Retorna o número de carros na lista
- `quit` - Sai do cliente

## Estrutura do Projeto
```
.
├── include/          # Ficheiros de cabeçalho (.h)
├── source/           # Ficheiros de implementação (.c)
├── object/           # Ficheiros objeto (.o) - gerados pela compilação
├── lib/              # Bibliotecas (.a) - geradas pela compilação
├── binary/           # Executáveis - gerados pela compilação
├── Makefile          # Ficheiro de compilação
└── README.md         # Este ficheiro
```

## Dependências
- GCC (compilador C)
- protobuf-c (biblioteca Protocol Buffers para C)

### Instalação das dependências no macOS
```bash
brew install protobuf-c
```

### Instalação das dependências no Linux (Ubuntu/Debian)
```bash
sudo apt-get install libprotobuf-c-dev protobuf-c-compiler
```

## Notas de Implementação

### sdmessage.pb-c.c
Este ficheiro contém as implementações das funções de serialização e deserialização do Protocol Buffers.
As funções principais incluem:
- `data__init()` - Inicializa estrutura Data
- `data__pack()` - Serializa Data
- `data__unpack()` - Deserializa Data
- `message_t__init()` - Inicializa estrutura MessageT
- `message_t__pack()` - Serializa MessageT
- `message_t__unpack()` - Deserializa MessageT

Também define os descritores necessários para o Protocol Buffers.

### list_server.c
O servidor:
1. Valida os argumentos da linha de comandos
2. Ignora o sinal SIGPIPE para evitar crashes
3. Inicializa a lista de carros através do skeleton
4. Cria um socket TCP e fica à escuta na porta especificada
5. Entra num loop infinito atendendo clientes (um de cada vez)
6. Para cada cliente: recebe pedido, processa através do skeleton, envia resposta
7. Liberta recursos quando terminado

### Protocol Buffers
O ficheiro `sdmessage.proto` define:
- Enumerações para Marca e Combustivel
- Estrutura Data (representa um carro)
- Estrutura message_t (mensagem de comunicação)
- Opcodes para operações (OP_ADD, OP_GET, OP_DEL, etc.)
- Tipos de conteúdo (CT_DATA, CT_MARCA, CT_YEAR, etc.)

## Implementação do Cliente

O cliente (`list_client.c`) oferece uma interface interativa que:
- Valida e processa comandos do utilizador
- Converte entre formatos de dados (strings ↔ enums)
- Apresenta resultados de forma legível
- Trata erros de forma apropriada
- Oferece comando `help` para assistência

### Funcionalidades adicionais do cliente:
- **Validação de entrada**: Verifica sintaxe dos comandos
- **Mensagens de erro claras**: Indica uso correto quando há erro
- **Formatação de saída**: Apresenta dados de forma estruturada
- **Gestão de memória**: Liberta recursos corretamente

## Implementação Adicional

### list_get_all()
Foi implementada a função `list_get_all()` em `list.c` que:
- Retorna um array de ponteiros para TODOS os carros da lista
- O array é terminado com NULL
- Os ponteiros apontam para dados internos (não duplicados)
- Apenas o array deve ser libertado pelo chamador

Esta função é usada pelo servidor no comando `get_list_ordered_by_year`.

## Limitações Conhecidas
- O servidor apenas atende um cliente de cada vez (concorrência será implementada no projeto 3)
- Não há persistência de dados (a lista é mantida apenas em memória)
- Os warnings de conversão de enum durante a compilação são esperados (conversão entre enums do protobuf e enums do projeto)

## Testes
Para testar o sistema:

### Teste básico:
1. **Terminal 1 - Servidor:**
   ```bash
   ./binary/list_server 12345
   ```

2. **Terminal 2 - Cliente:**
   ```bash
   ./binary/list_client localhost:12345
   ```

3. **No cliente, experimente os comandos:**
   ```
   >>> help
   >>> add 2020 25000.50 TOYOTA Corolla GASOLINA
   >>> add 2021 35000.00 BMW X5 GASOLEO
   >>> add 2020 28000.00 AUDI A4 HIBRIDO
   >>> size
   >>> get_model_list
   >>> get_by_marca TOYOTA
   >>> get_by_year 2020
   >>> get_list_ordered_by_year
   >>> remove Corolla
   >>> size
   >>> quit
   ```

### Teste de interoperabilidade:
Também podem ser utilizados os binários fornecidos pelos docentes para testar a interoperabilidade:
- Teste o vosso cliente com o servidor dos docentes
- Teste o vosso servidor com o cliente dos docentes

### Teste de robustez:
- Teste comandos inválidos
- Teste desconexão abrupta (Ctrl+C no servidor/cliente)
- Teste com múltiplas conexões sequenciais
