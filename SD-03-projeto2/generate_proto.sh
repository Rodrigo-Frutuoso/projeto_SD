#!/bin/bash
# Script para gerar código Protocol Buffers

echo "Gerando código Protocol Buffers..."

# Verificar se protoc-c está instalado
if ! command -v protoc-c &> /dev/null; then
    echo "ERRO: protoc-c não está instalado!"
    echo ""
    echo "Para instalar no macOS:"
    echo "  brew install protobuf-c"
    echo ""
    echo "Para instalar no Linux (Ubuntu/Debian):"
    echo "  sudo apt-get install protobuf-c-compiler libprotobuf-c-dev"
    echo ""
    echo "Para instalar no Linux (Fedora/RHEL):"
    echo "  sudo dnf install protobuf-c-compiler protobuf-c-devel"
    exit 1
fi

# Gerar código
protoc-c --c_out=. ./sdmessage.proto

# Mover ficheiros para as pastas corretas
if [ -f "sdmessage.pb-c.c" ] && [ -f "sdmessage.pb-c.h" ]; then
    mv sdmessage.pb-c.c source/
    mv sdmessage.pb-c.h include/
    echo "Código Protocol Buffers gerado com sucesso!"
    echo "  - source/sdmessage.pb-c.c"
    echo "  - include/sdmessage.pb-c.h"
else
    echo "ERRO: Falha ao gerar código Protocol Buffers"
    exit 1
fi
