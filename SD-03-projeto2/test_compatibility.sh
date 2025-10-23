#!/bin/bash

###############################################################################
# Script de Teste de Compatibilidade - Projeto SD Grupo 03
# 
# Testa todas as 4 combinações possíveis:
# 1. Cliente Estudante + Servidor Estudante
# 2. Cliente Professor + Servidor Estudante  
# 3. Cliente Estudante + Servidor Professor
# 4. Cliente Professor + Servidor Professor
#
# Uso: ./test_compatibility.sh
###############################################################################

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# Configuração
PORT=12345
TIMEOUT=5
TEST_FILE="test_commands_compat.txt"

# Criar ficheiro de comandos de teste se não existir
if [ ! -f "$TEST_FILE" ]; then
    cat > "$TEST_FILE" << 'EOF'
add Corolla 2020 25000 0 0
add 120d 2010 35000 1 1
add Clio 2019 18000 2 0
size
get_model_list
remove Clio
size
quit
EOF
    echo -e "${GREEN}✓${NC} Ficheiro de teste criado: $TEST_FILE"
fi

# Função para imprimir cabeçalho
print_header() {
    echo ""
    echo -e "${BOLD}═══════════════════════════════════════════════════════════════${NC}"
    echo -e "${BOLD}  $1${NC}"
    echo -e "${BOLD}═══════════════════════════════════════════════════════════════${NC}"
}

# Função para imprimir subtítulo
print_subtitle() {
    echo -e "${BLUE}${BOLD}$1${NC}"
    echo -e "${BLUE}───────────────────────────────────────────────────────────────${NC}"
}

# Função para matar processos na porta
cleanup_port() {
    lsof -ti:$PORT | xargs kill -9 2>/dev/null
    sleep 0.5
}

# Função para executar teste
run_test() {
    local test_num=$1
    local test_name=$2
    local server_bin=$3
    local client_bin=$4
    local output_file="test${test_num}_output.txt"
    
    print_subtitle "Teste $test_num: $test_name"
    echo -e "Servidor: ${YELLOW}$server_bin${NC}"
    echo -e "Cliente:  ${YELLOW}$client_bin${NC}"
    echo ""
    
    # Limpar porta
    cleanup_port
    
    # Iniciar servidor em background
    echo -e "${BLUE}→${NC} A iniciar servidor..."
    $server_bin $PORT > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
    
    # Verificar se servidor iniciou
    if ! kill -0 $SERVER_PID 2>/dev/null; then
        echo -e "${RED}✗ FALHOU${NC} - Servidor não iniciou"
        echo ""
        return 1
    fi
    
    echo -e "${GREEN}✓${NC} Servidor a correr (PID: $SERVER_PID)"
    
    # Executar cliente
    echo -e "${BLUE}→${NC} A executar cliente..."
    
    timeout $TIMEOUT $client_bin 127.0.0.1:$PORT < "$TEST_FILE" > "$output_file" 2>&1
    CLIENT_EXIT=$?
    
    # Aguardar um pouco
    sleep 0.5
    
    # Analisar resultado
    if [ $CLIENT_EXIT -eq 0 ]; then
        # Verificar se tem output esperado
        if grep -q "List size:" "$output_file" && grep -q "adicionado com sucesso" "$output_file"; then
            echo -e "${GREEN}✓${NC} Cliente executou com sucesso"
            echo -e "${GREEN}${NC}"
            RESULT="PASS"
        else
            echo -e "${YELLOW}⚠${NC} Cliente terminou mas output pode estar incorreto"
            echo -e "${YELLOW}~${NC}"
            RESULT="WARN"
        fi
    elif [ $CLIENT_EXIT -eq 124 ]; then
        echo -e "${RED}✗${NC} Cliente ficou bloqueado (timeout)"
        echo -e "${RED}✗ FALHOU${NC}"
        RESULT="FAIL"
    else
        echo -e "${RED}✗${NC} Cliente terminou com erro (código: $CLIENT_EXIT)"
        echo -e "${RED}✗ FALHOU${NC}"
        RESULT="FAIL"
    fi
    
    # Matar servidor
    kill -9 $SERVER_PID 2>/dev/null
    wait $SERVER_PID 2>/dev/null
    
    echo -e "${BLUE}→${NC} Output guardado em: ${YELLOW}$output_file${NC}"
    echo ""
    
    return 0
}

# Início dos testes
clear
print_header "TESTE DE COMPATIBILIDADE - PROJETO SD GRUPO 03"

echo -e "${BOLD}Data:${NC} $(date '+%Y-%m-%d %H:%M:%S')"
echo -e "${BOLD}Porta de teste:${NC} $PORT"
echo -e "${BOLD}Timeout:${NC} $TIMEOUT segundos"
echo ""

# Array para guardar resultados
declare -a RESULTS

# Teste 1: Cliente Estudante + Servidor Estudante
run_test 1 "Cliente Estudante + Servidor Estudante" "./binary/list_server" "./binary/list_client"
RESULTS[1]=$RESULT

# Teste 2: Cliente Professor + Servidor Estudante
run_test 2 "Cliente Professor + Servidor Estudante" "./binary/list_server" "./binary/list_client_profs"
RESULTS[2]=$RESULT

# Teste 3: Cliente Estudante + Servidor Professor
run_test 3 "Cliente Estudante + Servidor Professor" "./binary/list_server_profs" "./binary/list_client"
RESULTS[3]=$RESULT

# Teste 4: Cliente Professor + Servidor Professor (Baseline)
run_test 4 "Cliente Professor + Servidor Professor" "./binary/list_server_profs" "./binary/list_client_profs"
RESULTS[4]=$RESULT

# Limpar porta final
cleanup_port

# Comparar outputs automaticamente
print_header "COMPARAÇÃO AUTOMÁTICA DE OUTPUTS"

echo -e "${BOLD}Comparando Test 1 (Estudante+Estudante) vs Test 4 (Professor+Professor):${NC}"
if diff -q test1_output.txt test4_output.txt > /dev/null 2>&1; then
    echo -e "${GREEN}✓ IDÊNTICOS${NC} - Outputs são exatamente iguais!"
else
    echo -e "${YELLOW}⚠ DIFERENÇAS ENCONTRADAS${NC}"
    echo -e "\nDiferenças:"
    diff --color=auto test1_output.txt test4_output.txt || true
fi
echo ""

echo -e "${BOLD}Comparando Test 2 (Professor+Estudante) vs Test 4 (Baseline):${NC}"
if diff -q test2_output.txt test4_output.txt > /dev/null 2>&1; then
    echo -e "${GREEN}✓ IDÊNTICOS${NC} - Outputs são exatamente iguais!"
else
    echo -e "${YELLOW}⚠ DIFERENÇAS ENCONTRADAS:${NC}"
    echo ""
    # Mostrar diferenças linha a linha
    diff --color=always -y --suppress-common-lines test2_output.txt test4_output.txt | head -20
    echo ""
    DIFF_COUNT=$(diff test2_output.txt test4_output.txt | grep "^[<>]" | wc -l)
    echo -e "${YELLOW}  Total: ${DIFF_COUNT} linha(s) diferente(s)${NC}"
fi
echo ""


echo -e "${BOLD}Ficheiros de output gerados:${NC}"
echo -e "  • test1_output.txt - Cliente Estudante + Servidor Estudante"
echo -e "  • test2_output.txt - Cliente Professor + Servidor Estudante"
echo -e "  • test3_output.txt - Cliente Estudante + Servidor Professor"
echo -e "  • test4_output.txt - Cliente Professor + Servidor Professor"
echo ""