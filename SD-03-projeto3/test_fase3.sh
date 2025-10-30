#!/bin/bash
# Grupo 03
# Script de teste para Fase 3

echo "=== Teste da Fase 3 - Servidor Multi-threaded ==="
echo ""

# Limpar log antigo
rm -f server.log

# Iniciar servidor
echo "1. Iniciando servidor na porta 12345..."
./binary/list_server 12345 &
SERVER_PID=$!
sleep 1

# Testar cliente normal (deve ser aceite)
echo ""
echo "2. Testando cliente normal (deve ser aceite - OP_READY)..."
(echo "size"; sleep 1; echo "quit") | ./binary/list_client 127.0.0.1:12345 &
CLIENT1_PID=$!
sleep 2

# Verificar se log foi criado
echo ""
echo "3. Verificando ficheiro server.log..."
if [ -f server.log ]; then
    echo "✅ Ficheiro server.log criado!"
    echo ""
    echo "Conteúdo do log:"
    cat server.log
else
    echo "❌ ERRO: Ficheiro server.log não foi criado!"
fi

# Terminar servidor
echo ""
echo "4. Terminando servidor..."
kill $SERVER_PID 2>/dev/null
sleep 1

echo ""
echo "=== Teste concluído ==="
echo ""
echo "Para testar limite de 5 clientes:"
echo "  1. Inicie o servidor: ./binary/list_server 12345"
echo "  2. Em 5 terminais diferentes, execute: ./binary/list_client 127.0.0.1:12345"
echo "  3. No 6º terminal, deve aparecer: 'Server busy. Try again later.'"
