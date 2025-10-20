#!/bin/bash

echo "========================================================================"
echo "🔍 TESTE DE MEMORY LEAKS COM VALGRIND"
echo "========================================================================"
echo ""

# Verificar se valgrind está instalado
if ! command -v valgrind &> /dev/null; then
    echo "❌ valgrind não está instalado!"
    echo "Para instalar: sudo apt install valgrind"
    exit 1
fi

echo "✅ valgrind encontrado"
echo ""

# Matar servidores antigos
pkill -f list_server 2>/dev/null
sleep 1

# Compilar em modo debug
echo "Recompilando em modo debug..."
make clean > /dev/null 2>&1
make all > /dev/null 2>&1

if [ $? -ne 0 ]; then
    echo "❌ Erro na compilação!"
    exit 1
fi

echo "✅ Compilação OK"
echo ""

# ============================================================================
# TESTE 1: CLIENTE
# ============================================================================
echo "========================================================================"
echo "TESTE 1: MEMORY LEAKS NO CLIENTE"
echo "========================================================================"
echo ""

# Iniciar servidor normal (sem valgrind para ser mais rápido)
./binary/list_server 12345 > /tmp/server_test.log 2>&1 &
SERVER_PID=$!
sleep 2

if ! ps -p $SERVER_PID > /dev/null; then
    echo "❌ Servidor não iniciou!"
    exit 1
fi

echo "✅ Servidor iniciado (PID: $SERVER_PID)"
echo ""

# Testar cliente com valgrind
echo "Executando cliente com valgrind (pode demorar ~10 segundos)..."
echo ""

echo -e "add 2020 25000 TOYOTA Corolla GASOLINA\nadd 2021 30000 BMW X3 GASOLEO\nadd 2022 35000 AUDI A4 HIBRIDO\nget_model_list\nget_by_marca TOYOTA\nget_by_year 2020\nremove Corolla\nsize\nget_list_ordered_by_year\nquit" | \
valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --error-exitcode=1 \
    --log-file=/tmp/valgrind_client.log \
    ./binary/list_client localhost:12345 > /dev/null 2>&1

CLIENT_RESULT=$?

# Matar servidor
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "RESULTADOS DO CLIENTE:"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Extrair informação importante do log
if [ -f /tmp/valgrind_client.log ]; then
    echo ""
    echo "HEAP SUMMARY:"
    grep "HEAP SUMMARY" /tmp/valgrind_client.log -A 3
    echo ""

    echo "LEAK SUMMARY:"
    grep "LEAK SUMMARY" /tmp/valgrind_client.log -A 5
    echo ""

    # Verificar se há leaks
    if grep -q "All heap blocks were freed -- no leaks are possible" /tmp/valgrind_client.log; then
        echo "✅ CLIENTE: NENHUM MEMORY LEAK DETECTADO!"
        echo "   (All heap blocks were freed)"
    else
        DEFINITELY_LOST=$(grep "definitely lost:" /tmp/valgrind_client.log | awk '{print $4}' | sed 's/,//g')
        INDIRECTLY_LOST=$(grep "indirectly lost:" /tmp/valgrind_client.log | awk '{print $4}' | sed 's/,//g')

        if [ "$DEFINITELY_LOST" = "0" ] && [ "$INDIRECTLY_LOST" = "0" ]; then
            echo "✅ CLIENTE: NENHUM MEMORY LEAK DETECTADO!"
        else
            echo "⚠️  CLIENTE: Memory leaks detectados!"
            echo ""
            echo "Detalhes completos em: /tmp/valgrind_client.log"
            echo ""
            echo "LEAKS ENCONTRADOS:"
            grep "definitely lost" /tmp/valgrind_client.log
            grep "indirectly lost" /tmp/valgrind_client.log
        fi
    fi
else
    echo "❌ Erro: Log do valgrind não foi criado!"
fi

echo ""

# ============================================================================
# TESTE 2: SERVIDOR (teste curto)
# ============================================================================
echo "========================================================================"
echo "TESTE 2: MEMORY LEAKS NO SERVIDOR (teste rápido)"
echo "========================================================================"
echo ""

echo "Iniciando servidor com valgrind..."
echo "(Servidor vai processar algumas requisições e depois será terminado)"
echo ""

# Iniciar servidor com valgrind
timeout 10 valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --log-file=/tmp/valgrind_server.log \
    ./binary/list_server 12345 > /dev/null 2>&1 &

SERVER_PID=$!
sleep 3

# Fazer algumas operações
echo -e "add 2020 25000 TOYOTA Corolla GASOLINA\nquit" | ./binary/list_client localhost:12345 > /dev/null 2>&1
sleep 1
echo -e "add 2021 30000 BMW X3 GASOLEO\nget_model_list\nquit" | ./binary/list_client localhost:12345 > /dev/null 2>&1
sleep 1
echo -e "size\nquit" | ./binary/list_client localhost:12345 > /dev/null 2>&1
sleep 1

# Terminar servidor
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "RESULTADOS DO SERVIDOR:"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ -f /tmp/valgrind_server.log ]; then
    echo ""
    echo "HEAP SUMMARY:"
    grep "HEAP SUMMARY" /tmp/valgrind_server.log -A 3
    echo ""

    echo "LEAK SUMMARY:"
    grep "LEAK SUMMARY" /tmp/valgrind_server.log -A 5
    echo ""

    # Verificar se há leaks
    if grep -q "All heap blocks were freed -- no leaks are possible" /tmp/valgrind_server.log; then
        echo "✅ SERVIDOR: NENHUM MEMORY LEAK DETECTADO!"
        echo "   (All heap blocks were freed)"
    else
        DEFINITELY_LOST=$(grep "definitely lost:" /tmp/valgrind_server.log | awk '{print $4}' | sed 's/,//g')
        INDIRECTLY_LOST=$(grep "indirectly lost:" /tmp/valgrind_server.log | awk '{print $4}' | sed 's/,//g')

        if [ "$DEFINITELY_LOST" = "0" ] && [ "$INDIRECTLY_LOST" = "0" ]; then
            echo "✅ SERVIDOR: NENHUM MEMORY LEAK DETECTADO!"
            echo ""
            echo "NOTA: 'still reachable' são buffers de bibliotecas (normal)"
        else
            echo "⚠️  SERVIDOR: Memory leaks detectados!"
            echo ""
            echo "Detalhes completos em: /tmp/valgrind_server.log"
            echo ""
            echo "LEAKS ENCONTRADOS:"
            grep "definitely lost" /tmp/valgrind_server.log
            grep "indirectly lost" /tmp/valgrind_server.log
        fi
    fi
else
    echo "❌ Erro: Log do valgrind não foi criado!"
fi

echo ""
echo "========================================================================"
echo "📊 RESUMO FINAL"
echo "========================================================================"
echo ""

# Verificar ambos
CLIENT_LEAKS=0
SERVER_LEAKS=0

if [ -f /tmp/valgrind_client.log ]; then
    if ! grep -q "All heap blocks were freed" /tmp/valgrind_client.log; then
        CLIENT_DEF=$(grep "definitely lost:" /tmp/valgrind_client.log | awk '{print $4}' | sed 's/,//g')
        CLIENT_IND=$(grep "indirectly lost:" /tmp/valgrind_client.log | awk '{print $4}' | sed 's/,//g')
        if [ "$CLIENT_DEF" != "0" ] || [ "$CLIENT_IND" != "0" ]; then
            CLIENT_LEAKS=1
        fi
    fi
fi

if [ -f /tmp/valgrind_server.log ]; then
    if ! grep -q "All heap blocks were freed" /tmp/valgrind_server.log; then
        SERVER_DEF=$(grep "definitely lost:" /tmp/valgrind_server.log | awk '{print $4}' | sed 's/,//g')
        SERVER_IND=$(grep "indirectly lost:" /tmp/valgrind_server.log | awk '{print $4}' | sed 's/,//g')
        if [ "$SERVER_DEF" != "0" ] || [ "$SERVER_IND" != "0" ]; then
            SERVER_LEAKS=1
        fi
    fi
fi

if [ $CLIENT_LEAKS -eq 0 ] && [ $SERVER_LEAKS -eq 0 ]; then
    echo "╔═══════════════════════════════════════════════════════════════╗"
    echo "║                                                               ║"
    echo "║   ✅ EXCELENTE! NENHUM MEMORY LEAK DETECTADO! ✅              ║"
    echo "║                                                               ║"
    echo "║   O projeto tem gestão de memória PERFEITA! 🎉               ║"
    echo "║                                                               ║"
    echo "╚═══════════════════════════════════════════════════════════════╝"
    echo ""
    exit 0
else
    echo "╔═══════════════════════════════════════════════════════════════╗"
    echo "║                                                               ║"
    echo "║   ⚠️  ATENÇÃO: Memory leaks detectados                       ║"
    echo "║                                                               ║"
    echo "╚═══════════════════════════════════════════════════════════════╝"
    echo ""
    echo "Verifique os logs detalhados:"
    echo "  - Cliente: /tmp/valgrind_client.log"
    echo "  - Servidor: /tmp/valgrind_server.log"
    echo ""
    exit 1
fi
