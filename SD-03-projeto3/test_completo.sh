#!/bin/bash
# Grupo 03
# Teste completo da funcionalidade multi-threading

echo "=== TESTE COMPLETO - FASE 3 ==="
echo ""

# Limpar logs
rm -f server.log

# Iniciar servidor
echo "[1/5] Iniciando servidor..."
./binary/list_server 12345 > /dev/null 2>&1 &
SERVER_PID=$!
sleep 1

# Criar comandos de teste para clientes
TEST_COMMANDS=$(cat << 'EOF'
add Corolla 2020 25000 0 0
add Civic 2021 28000 1 0
size
get_model_list
quit
EOF
)

# Lançar 3 clientes simultâneos
echo "[2/5] Lançando 3 clientes simultâneos..."
for i in 1 2 3; do
    echo "$TEST_COMMANDS" | ./binary/list_client 127.0.0.1:12345 > /dev/null 2>&1 &
    sleep 0.2
done

# Aguardar clientes terminarem
sleep 3

# Verificar log
echo "[3/5] Verificando server.log..."
if [ -f server.log ]; then
    echo "✅ Log criado com sucesso!"
    echo ""

    CONNECTS=$(grep -c "CONNECT" server.log)
    REQUESTS=$(grep -c "REQUEST" server.log)
    CLOSES=$(grep -c "CLOSE" server.log)

    echo "📊 Estatísticas:"
    echo "   - Conexões: $CONNECTS"
    echo "   - Pedidos: $REQUESTS"
    echo "   - Fechos: $CLOSES"
    echo ""

    echo "📝 Primeiras 10 linhas do log:"
    head -10 server.log
else
    echo "❌ ERRO: Log não foi criado!"
fi

# Testar limite de clientes
echo ""
echo "[4/5] Testando limite de 5 clientes..."
echo "Lançando 6 clientes simultâneos..."

# Lançar 5 clientes que ficam ativos
for i in 1 2 3 4 5; do
    (sleep 10; echo "quit") | ./binary/list_client 127.0.0.1:12345 > /dev/null 2>&1 &
    sleep 0.1
done

# Aguardar um pouco para garantir que estão conectados
sleep 1

# Tentar 6º cliente (deve ser rejeitado)
echo "Tentando conectar 6º cliente (deve ser rejeitado)..."
RESULT=$(echo "quit" | ./binary/list_client 127.0.0.1:12345 2>&1)

if echo "$RESULT" | grep -q "Server busy"; then
    echo "✅ Servidor corretamente rejeitou 6º cliente!"
    echo "   Mensagem: 'Server busy. Try again later.'"
else
    echo "⚠️  Comportamento inesperado do 6º cliente"
fi

# Terminar tudo
echo ""
echo "[5/5] Limpando processos..."
pkill -f list_client
sleep 1
kill $SERVER_PID
sleep 1

echo ""
echo "=== TESTE CONCLUÍDO ==="
echo ""
echo "O ficheiro server.log contém todos os eventos registados."
echo "Execute 'cat server.log' para ver o conteúdo completo."
