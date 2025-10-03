# SD-03-projeto1

Grupo 03
- Rodrigo Frutuoso - 61865
- Simão Alexandre - 61874
- Tiago Leite - 61863

Projeto 1 — Sistemas Distribuídos 2025/2026

Este projeto implementa:
- `data_t` e operações (criar, destruir, duplicar, substituir)
- `list_t` (lista ligada simples) com operações: criar, destruir, adicionar, remover por modelo, procurar por marca, obter por ano, ordenar por ano, obter lista de modelos
- `serialization` para serializar/desserializar um carro e uma lista de carros

Notas de implementação
- Serialização usa network byte order (big-endian) para todos os inteiros (ano, marca, combustível, comprimentos e número de elementos).
- O campo `preco` (float) é copiado tal como está; para interoperabilidade entre arquiteturas diferentes, recomendar-se-ia usar um formato portátil (por exemplo, conversão para string), mas não é exigido no enunciado.
- A ordenação por ano é estável o suficiente para os testes, usando bubble sort sobre os dados dos nós.

Estrutura de diretórios
- `include/` headers
- `source/` código C
- `object/` objetos (gerados pelo make)
- `binary/` executáveis de teste (gerados pelo make)
- `tests/` programas de teste fornecidos

Como compilar e testar

```bash
make clean
make
make test
```

Os executáveis de teste são criados em `binary/`: `test_data`, `test_list`, `test_serialization`.

Limitações/conhecidas
- Sem verificação de overflows ao construir buffers de serialização (tamanhos vêm dos dados); confia-se em entradas razoáveis.
- `list_get_model_list` duplica strings; usar `list_free_model_list` para libertar corretamente.
