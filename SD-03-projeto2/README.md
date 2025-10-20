# SD-03-projeto2

Grupo 03
- Rodrigo Frutuoso - 61865
- Simão Alexandre - 61874
- Tiago Leite - 61863

Projeto 2 — Sistemas Distribuídos 2025/2026

Este projeto implementa um sistema cliente-servidor usando Protocol Buffers:
- `list_client` - cliente interativo com comandos para gerir inventário de carros
- `list_server` - servidor que mantém a lista em memória e atende pedidos
- `client_stub.c/h` - stub RPC do lado do cliente
- `network_client.c/h` - comunicação TCP do cliente
- `list_skel.c/h` - skeleton RPC do lado do servidor
- `network_server.c/h` - comunicação TCP do servidor
- `message-private.c/h` - funções auxiliares para envio/receção de mensagens

## Notas de implementação
- Usa Protocol Buffers (sdmessage.proto fornecido) para serialização/desserialização
- Servidor atende um cliente de cada vez (concorrência será implementada no projeto 3)
- Antes de enviar/receber mensagem, envia-se/recebe-se um inteiro de 4 bytes (network byte order) com o tamanho da mensagem
- Implementadas funções `read_all()` e `write_all()` para garantir leitura/escrita completa
- Servidor ignora SIGPIPE e usa SO_REUSEADDR para reinicialização rápida
- Usa ficheiros objeto `data.o` e `list.o` fornecidos pelos docentes

## Estrutura de diretórios
- `include/` headers
- `source/` código C
- `object/` objetos (gerados pelo make, exceto data.o e list.o fornecidos)
- `lib/` biblioteca liblist.a (gerada pelo make)
- `binary/` executáveis (gerados pelo make)

## Como compilar e executar

```bash
make clean
make all
```

Servidor:
```bash
./binary/list_server <porta>
```

Cliente (noutro terminal):
```bash
./binary/list_client <servidor>:<porta>
```

Exemplo:
```bash
./binary/list_server 12345
./binary/list_client localhost:12345
```

## Comandos disponíveis no cliente
- `add <ano> <preco> <marca> <modelo> <combustivel>` - adiciona carro
- `remove <modelo>` - remove carro pelo modelo
- `get_by_marca <marca>` - obtém carro pela marca
- `get_by_year <ano>` - obtém carros de um ano específico
- `get_list_ordered_by_year` - obtém lista ordenada por ano
- `get_model_list` - obtém lista de modelos
- `size` - número de carros na lista
- `quit` - termina cliente

## Dependências
```bash
sudo apt-get install libprotobuf-c-dev protobuf-c-compiler
```
