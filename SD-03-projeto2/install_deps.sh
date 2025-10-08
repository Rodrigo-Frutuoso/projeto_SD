#!/bin/bash
# Script de instalação de dependências

echo "=== Instalação de Dependências - Projeto SD ==="
echo ""

# Detectar sistema operacional
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    echo "Sistema detectado: macOS"
    echo "Instalando protobuf-c via Homebrew..."

    if ! command -v brew &> /dev/null; then
        echo "ERRO: Homebrew não está instalado!"
        echo "Instale em: https://brew.sh"
        exit 1
    fi

    brew install protobuf-c

elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    echo "Sistema detectado: Linux"

    if command -v apt-get &> /dev/null; then
        echo "Instalando via apt-get..."
        sudo apt-get update
        sudo apt-get install -y protobuf-c-compiler libprotobuf-c-dev
    elif command -v dnf &> /dev/null; then
        echo "Instalando via dnf..."
        sudo dnf install -y protobuf-c-compiler protobuf-c-devel
    elif command -v yum &> /dev/null; then
        echo "Instalando via yum..."
        sudo yum install -y protobuf-c-compiler protobuf-c-devel
    else
        echo "ERRO: Gestor de pacotes não suportado"
        exit 1
    fi
else
    echo "ERRO: Sistema operacional não suportado: $OSTYPE"
    exit 1
fi

echo ""
echo "=== Dependências instaladas com sucesso! ==="
echo ""
echo "Próximos passos:"
echo "  1. ./generate_proto.sh   # Gerar código Protocol Buffers"
echo "  2. make all              # Compilar projeto"
