# NOTAS IMPORTANTES - ANTES DE COMPILAR

## 1. Instalar Protocol Buffers C

O projeto requer `protobuf-c` para compilar. Instale usando:

```bash
./install_deps.sh
```

Ou manualmente:
- **macOS:** `brew install protobuf-c`
- **Ubuntu/Debian:** `sudo apt-get install protobuf-c-compiler libprotobuf-c-dev`
- **Fedora/RHEL:** `sudo dnf install protobuf-c-compiler protobuf-c-devel`

## 2. Gerar Código Protocol Buffers

Antes de compilar, é necessário gerar o ficheiro `sdmessage.pb-c.c`:

```bash
./generate_proto.sh
```

Este script irá:
- Compilar `sdmessage.proto` usando `protoc-c`
- Gerar `source/sdmessage.pb-c.c` e `include/sdmessage.pb-c.h`

**NOTA:** O ficheiro `include/sdmessage.pb-c.h` já foi fornecido pelos professores, mas o `.c` precisa ser gerado.

## 3. Compilar o Projeto

```bash
make all
```

Isto criará:
- `lib/liblist.a` - Biblioteca com data.o e list.o
- `binary/list_client` - Executável do cliente
- `binary/list_server` - Executável do servidor

## 4. Executar

**Terminal 1 (Servidor):**
```bash
./binary/list_server 8080
```

**Terminal 2 (Cliente):**
```bash
./binary/list_client localhost:8080
```

## Resolução de Problemas

### Erro: "protoc-c: command not found"
- Instale `protobuf-c` usando `./install_deps.sh`

### Erro: "cannot find -lprotobuf-c"
- A biblioteca não está instalada. Execute `./install_deps.sh`
- Ou ajuste `LIBS` no Makefile para apontar para a localização correta

### Erro de compilação em sdmessage.pb-c.c
- Verifique se executou `./generate_proto.sh` antes de `make`
- Verifique se a versão de `protoc-c` é compatível

### Warning: "implicit declaration of function"
- Verifique se todos os headers estão incluídos
- Compile com `make clean && make all`

## Estrutura Final

Após compilação bem-sucedida:
```
SD-03-projeto2/
├── Makefile
├── README.md
├── IMPLEMENTACAO.md
├── NOTAS.md
├── generate_proto.sh
├── install_deps.sh
├── sdmessage.proto
├── binary/
│   ├── list_client      ← Executável do cliente
│   └── list_server      ← Executável do servidor
├── include/
│   └── [11 ficheiros .h]
├── lib/
│   └── liblist.a        ← Biblioteca estática
├── object/
│   └── [9 ficheiros .o]
└── source/
    ├── [9 ficheiros .c implementados]
    └── sdmessage.pb-c.c ← Gerado por protoc-c
```

## Verificação

Para verificar se tudo está correto:

```bash
# Verificar se protoc-c está instalado
which protoc-c

# Verificar se código protobuf foi gerado
ls -l source/sdmessage.pb-c.c

# Verificar se compilou
ls -l binary/list_client binary/list_server

# Testar servidor
./binary/list_server 8080 &
SERVER_PID=$!

# Testar cliente (comandos de teste)
echo -e "size\nquit" | ./binary/list_client localhost:8080

# Parar servidor
kill $SERVER_PID
```
