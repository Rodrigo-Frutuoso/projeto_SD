#!/bin/bash

echo "========================================================================"
echo "🔍 TESTES EDGE CASE FINAIS"
echo "========================================================================"
echo ""

# Matar servidor antigo
pkill -f list_server 2>/dev/null
sleep 1

# Iniciar servidor
./binary/list_server 12345 > /tmp/edge_test.log 2>&1 &
SERVER_PID=$!
sleep 2

if ! ps -p $SERVER_PID > /dev/null; then
    echo "❌ Servidor não iniciou!"
    exit 1
fi

echo "✅ Servidor iniciado (PID: $SERVER_PID)"
echo ""

# Teste 1: Comandos inválidos
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "TESTE 1: Comandos inválidos devem dar erro claro"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "comando_invalido\nadd\nremove\nget_by_marca\nquit" | ./binary/list_client localhost:12345
echo ""

# Teste 2: Preços com decimais
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "TESTE 2: Preços com decimais diversos"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "add 2020 25000.99 TOYOTA Yaris GASOLINA\nadd 2021 0.01 RENAULT Clio GASOLEO\nadd 2022 999999.99 MERCEDES S-Class ELETRICO\nget_model_list\nquit" | ./binary/list_client localhost:12345
echo ""

# Teste 3: Nomes de modelos com caracteres especiais (só funciona com palavras simples por causa do sscanf)
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "TESTE 3: Remover múltiplos carros seguidos"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "add 2020 25000 TOYOTA Yaris GASOLINA\nadd 2021 30000 BMW X3 GASOLEO\nadd 2022 35000 AUDI A4 HIBRIDO\nremove Yaris\nremove X3\nremove A4\nsize\nquit" | ./binary/list_client localhost:12345
echo ""

# Teste 4: Get by marca inexistente
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "TESTE 4: Buscar marcas sem carros na lista"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "get_by_marca TOYOTA\nget_by_marca BMW\nget_by_marca MERCEDES\nquit" | ./binary/list_client localhost:12345
echo ""

# Teste 5: Ordenar lista vazia
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "TESTE 5: Ordenar lista vazia"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "get_list_ordered_by_year\nquit" | ./binary/list_client localhost:12345
echo ""

# Teste 6: Adicionar muitos carros e verificar ordenação
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "TESTE 6: Ordenação com anos desordenados"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "add 2025 60000 MERCEDES EQC ELETRICO\nadd 1990 5000 TOYOTA Corolla GASOLINA\nadd 2015 25000 BMW Serie3 GASOLEO\nadd 2000 10000 RENAULT Clio GASOLINA\nadd 2010 20000 AUDI A3 GASOLINA\nget_list_ordered_by_year\nquit" | ./binary/list_client localhost:12345
echo ""

# Teste 7: Múltiplas conexões simultâneas (simular)
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "TESTE 7: Persistência - Adicionar em conexão 1, buscar em conexão 2"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Conexão 1: Adicionar carros"
echo -e "add 2020 25000 TOYOTA Corolla GASOLINA\nadd 2021 30000 BMW X3 GASOLEO\nsize\nquit" | ./binary/list_client localhost:12345
echo ""
echo "Conexão 2: Verificar se persistiram"
echo -e "size\nget_model_list\nquit" | ./binary/list_client localhost:12345
echo ""

# Teste 8: All marcas e combustíveis
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "TESTE 8: Todas as marcas e combustíveis"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "add 2020 25000 TOYOTA Yaris GASOLINA\nadd 2020 40000 BMW X5 GASOLEO\nadd 2020 18000 RENAULT Clio HIBRIDO\nadd 2020 35000 AUDI A4 ELETRICO\nadd 2020 50000 MERCEDES C-Class GASOLINA\nget_by_marca TOYOTA\nget_by_marca BMW\nget_by_marca RENAULT\nget_by_marca AUDI\nget_by_marca MERCEDES\nquit" | ./binary/list_client localhost:12345
echo ""

# Matar servidor
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

echo "========================================================================"
echo "✅ TODOS OS TESTES EDGE CASE CONCLUÍDOS!"
echo "========================================================================"
echo ""
echo "Se todos os testes acima executaram sem crashes ou erros graves,"
echo "o projeto está 100% robusto e pronto para qualquer cenário!"
