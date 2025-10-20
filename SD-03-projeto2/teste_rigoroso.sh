#!/bin/bash

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

PASSED=0
FAILED=0

echo "======================================================================"
echo -e "${BLUE}🔬 BATERIA DE TESTES RIGOROSOS - PROJETO SD${NC}"
echo "======================================================================"
echo ""

# Matar servidores antigos
pkill -f list_server 2>/dev/null
sleep 1

# Iniciar servidor
echo -e "${YELLOW}[SETUP]${NC} Iniciando servidor na porta 12345..."
./binary/list_server 12345 > /tmp/server.log 2>&1 &
SERVER_PID=$!
sleep 2

if ! ps -p $SERVER_PID > /dev/null; then
    echo -e "${RED}❌ FALHA CRÍTICA: Servidor não iniciou!${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Servidor iniciado (PID: $SERVER_PID)${NC}"
echo ""

# Função para executar teste
run_test() {
    local test_name=$1
    local commands=$2
    local expected_pattern=$3

    echo -e "${YELLOW}[TEST]${NC} $test_name"

    result=$(echo -e "$commands\nquit" | timeout 5 ./binary/list_client localhost:12345 2>&1)

    if echo "$result" | grep -q "$expected_pattern"; then
        echo -e "${GREEN}  ✅ PASSOU${NC}"
        ((PASSED++))
        return 0
    else
        echo -e "${RED}  ❌ FALHOU${NC}"
        echo -e "${RED}     Esperado: $expected_pattern${NC}"
        echo -e "${RED}     Obtido: ${NC}"
        echo "$result" | tail -5 | sed 's/^/     /'
        ((FAILED++))
        return 1
    fi
}

echo "======================================================================"
echo -e "${BLUE}FASE 1: TESTES BÁSICOS${NC}"
echo "======================================================================"

# Teste 1: Conexão
run_test "1.1 - Conexão ao servidor" "" "Conectado com sucesso"

# Teste 2: Add único
run_test "1.2 - Adicionar um carro" \
    "add 2020 25000 TOYOTA Corolla GASOLINA" \
    "adicionado com sucesso"

# Teste 3: Size após add
run_test "1.3 - Verificar tamanho após adicionar" \
    "add 2021 30000 BMW X3 GASOLEO\nsize" \
    "Número de carros na lista: 1"

# Teste 4: Get by marca
run_test "1.4 - Buscar por marca existente" \
    "add 2020 25000 TOYOTA Corolla GASOLINA\nget_by_marca TOYOTA" \
    "TOYOTA"

# Teste 5: Remove existente
run_test "1.5 - Remover carro existente" \
    "add 2020 25000 TOYOTA Corolla GASOLINA\nremove Corolla" \
    "removido com sucesso"

echo ""
echo "======================================================================"
echo -e "${BLUE}FASE 2: TESTES DE ROBUSTEZ${NC}"
echo "======================================================================"

# Teste 6: Remove inexistente
run_test "2.1 - Remover carro inexistente" \
    "remove CarroInexistente" \
    "não encontrado"

# Teste 7: Get marca inexistente
run_test "2.2 - Buscar marca sem carros" \
    "add 2020 25000 TOYOTA Corolla GASOLINA\nget_by_marca BMW" \
    "não encontrado"

# Teste 8: Get ano sem carros
run_test "2.3 - Buscar ano sem carros" \
    "add 2020 25000 TOYOTA Corolla GASOLINA\nget_by_year 1999" \
    "Nenhum carro encontrado"

# Teste 9: Lista vazia
run_test "2.4 - Tamanho de lista vazia" \
    "size" \
    "Número de carros na lista: 0"

# Teste 10: Get list em lista vazia
run_test "2.5 - Listar modelos em lista vazia" \
    "get_model_list" \
    "Lista vazia"

echo ""
echo "======================================================================"
echo -e "${BLUE}FASE 3: TESTES COMPLEXOS (MÚLTIPLOS CARROS)${NC}"
echo "======================================================================"

# Teste 11: Múltiplas adições
run_test "3.1 - Adicionar 5 carros e verificar tamanho" \
    "add 2018 15000 TOYOTA Yaris GASOLINA
add 2019 20000 RENAULT Clio GASOLINA
add 2020 30000 AUDI A4 HIBRIDO
add 2021 40000 BMW X3 GASOLEO
add 2022 50000 MERCEDES C-Class ELETRICO
size" \
    "Número de carros na lista: 5"

# Teste 12: Get by year com múltiplos
run_test "3.2 - Buscar por ano com vários carros" \
    "add 2020 25000 TOYOTA Corolla GASOLINA
add 2020 30000 AUDI A4 HIBRIDO
add 2020 35000 BMW X1 GASOLEO
get_by_year 2020" \
    "Total: 3 carro(s) encontrado(s)"

# Teste 13: Get model list com vários
run_test "3.3 - Listar modelos (5 carros)" \
    "add 2018 15000 TOYOTA Yaris GASOLINA
add 2019 20000 RENAULT Clio GASOLINA
add 2020 30000 AUDI A4 HIBRIDO
add 2021 40000 BMW X3 GASOLEO
add 2022 50000 MERCEDES C-Class ELETRICO
get_model_list" \
    "Total: 5 modelo(s)"

# Teste 14: Ordenação
run_test "3.4 - Ordenar lista por ano" \
    "add 2022 50000 MERCEDES C-Class ELETRICO
add 2018 15000 TOYOTA Yaris GASOLINA
add 2020 30000 AUDI A4 HIBRIDO
add 2019 20000 RENAULT Clio GASOLINA
get_list_ordered_by_year" \
    "ordenada com sucesso"

# Teste 15: Remove do meio
run_test "3.5 - Remover carro do meio da lista" \
    "add 2018 15000 TOYOTA Yaris GASOLINA
add 2019 20000 RENAULT Clio GASOLINA
add 2020 30000 AUDI A4 HIBRIDO
remove Clio
size" \
    "Número de carros na lista: 2"

echo ""
echo "======================================================================"
echo -e "${BLUE}FASE 4: TESTES DE TODAS AS MARCAS E COMBUSTÍVEIS${NC}"
echo "======================================================================"

# Teste 16: Todas as marcas
run_test "4.1 - Todas as marcas (TOYOTA, BMW, RENAULT, AUDI, MERCEDES)" \
    "add 2020 25000 TOYOTA Corolla GASOLINA
add 2020 40000 BMW X3 GASOLEO
add 2020 18000 RENAULT Clio GASOLINA
add 2020 35000 AUDI A4 HIBRIDO
add 2020 50000 MERCEDES C-Class ELETRICO
get_model_list" \
    "Total: 5 modelo(s)"

# Teste 17: Todos os combustíveis
run_test "4.2 - Todos os combustíveis (GASOLINA, GASOLEO, HIBRIDO, ELETRICO)" \
    "add 2020 25000 TOYOTA Corolla GASOLINA
add 2020 30000 AUDI A3 GASOLEO
add 2020 35000 BMW X1 HIBRIDO
add 2020 40000 MERCEDES EQC ELETRICO
size" \
    "Número de carros na lista: 4"

echo ""
echo "======================================================================"
echo -e "${BLUE}FASE 5: TESTES DE STRESS (VOLUME)${NC}"
echo "======================================================================"

# Teste 18: Adicionar muitos carros
commands=""
for i in {1..20}; do
    commands+="add 202$((i%5)) $((20000 + i*1000)) TOYOTA Model$i GASOLINA\n"
done
commands+="size"

run_test "5.1 - Adicionar 20 carros e verificar tamanho" \
    "$commands" \
    "Número de carros na lista: 20"

# Teste 19: Listar muitos modelos
commands=""
for i in {1..15}; do
    commands+="add 2020 $((20000 + i*1000)) BMW Model$i GASOLEO\n"
done
commands+="get_model_list"

run_test "5.2 - Listar 15 modelos" \
    "$commands" \
    "Total: 15 modelo(s)"

echo ""
echo "======================================================================"
echo -e "${BLUE}FASE 6: TESTES DE SEQUÊNCIAS COMPLEXAS${NC}"
echo "======================================================================"

# Teste 20: Add, Remove, Add, Size
run_test "6.1 - Sequência: Add → Remove → Add → Size" \
    "add 2020 25000 TOYOTA Corolla GASOLINA
remove Corolla
add 2021 30000 BMW X3 GASOLEO
size" \
    "Número de carros na lista: 1"

# Teste 21: Múltiplos removes
run_test "6.2 - Remover múltiplos carros" \
    "add 2020 25000 TOYOTA Corolla GASOLINA
add 2021 30000 BMW X3 GASOLEO
add 2022 35000 AUDI A4 HIBRIDO
remove Corolla
remove X3
size" \
    "Número de carros na lista: 1"

# Teste 22: Busca após ordenação
run_test "6.3 - Buscar após ordenar lista" \
    "add 2022 50000 MERCEDES C-Class ELETRICO
add 2018 15000 TOYOTA Yaris GASOLINA
add 2020 30000 AUDI A4 HIBRIDO
get_list_ordered_by_year
get_by_marca TOYOTA" \
    "TOYOTA"

# Teste 23: Get by year após várias operações
run_test "6.4 - Get by year após add/remove" \
    "add 2020 25000 TOYOTA Corolla GASOLINA
add 2020 30000 AUDI A4 HIBRIDO
add 2020 35000 BMW X1 GASOLEO
remove Corolla
get_by_year 2020" \
    "Total: 2 carro(s) encontrado(s)"

echo ""
echo "======================================================================"
echo -e "${BLUE}FASE 7: TESTES DE PREÇOS E ANOS EXTREMOS${NC}"
echo "======================================================================"

# Teste 24: Preços muito altos
run_test "7.1 - Preço muito alto (999999.99)" \
    "add 2023 999999.99 MERCEDES S-Class ELETRICO
get_by_marca MERCEDES" \
    "999999"

# Teste 25: Preço muito baixo
run_test "7.2 - Preço muito baixo (100.00)" \
    "add 2000 100.00 RENAULT Clio GASOLINA
get_by_marca RENAULT" \
    "100"

# Teste 26: Anos diversos
run_test "7.3 - Anos diversos (1990 a 2025)" \
    "add 1990 5000 TOYOTA Corolla GASOLINA
add 2000 10000 BMW Serie3 GASOLEO
add 2010 20000 AUDI A3 GASOLINA
add 2020 40000 MERCEDES C-Class ELETRICO
add 2025 60000 RENAULT Megane HIBRIDO
size" \
    "Número de carros na lista: 5"

echo ""
echo "======================================================================"
echo -e "${BLUE}FASE 8: TESTE DE MÚLTIPLAS CONEXÕES${NC}"
echo "======================================================================"

# Teste 27: Verificar estado persistente entre conexões
echo -e "${YELLOW}[TEST]${NC} 8.1 - Persistência entre conexões (Conexão 1: Add)"
echo "add 2020 25000 TOYOTA Corolla GASOLINA\nquit" | timeout 5 ./binary/list_client localhost:12345 > /dev/null 2>&1

echo -e "${YELLOW}[TEST]${NC} 8.2 - Persistência entre conexões (Conexão 2: Verificar)"
result=$(echo "size\nquit" | timeout 5 ./binary/list_client localhost:12345 2>&1)

if echo "$result" | grep -q "Número de carros na lista: 1"; then
    echo -e "${GREEN}  ✅ PASSOU - Estado persistiu entre conexões${NC}"
    ((PASSED++))
else
    echo -e "${RED}  ❌ FALHOU - Estado não persistiu${NC}"
    ((FAILED++))
fi

echo ""
echo "======================================================================"
echo -e "${BLUE}📊 RESULTADOS FINAIS${NC}"
echo "======================================================================"
echo ""

TOTAL=$((PASSED + FAILED))
PERCENTAGE=$((PASSED * 100 / TOTAL))

echo -e "Total de testes: ${BLUE}$TOTAL${NC}"
echo -e "Testes passados: ${GREEN}$PASSED${NC}"
echo -e "Testes falhados: ${RED}$FAILED${NC}"
echo -e "Taxa de sucesso: ${GREEN}$PERCENTAGE%${NC}"
echo ""

# Matar servidor
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

if [ $FAILED -eq 0 ]; then
    echo "======================================================================"
    echo -e "${GREEN}🎉 PARABÉNS! TODOS OS TESTES PASSARAM!${NC}"
    echo -e "${GREEN}✅ O projeto está PERFEITO e pronto para entrega!${NC}"
    echo "======================================================================"
    exit 0
else
    echo "======================================================================"
    echo -e "${RED}⚠️  ATENÇÃO: Alguns testes falharam!${NC}"
    echo -e "${YELLOW}Revise os testes que falharam acima.${NC}"
    echo "======================================================================"
    exit 1
fi
