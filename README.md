# Escalonamento de Tarefas Críticas de Voo
 
Simulador em C que compara dois escalonadores de tempo real — rate-monotonic (`rate`)
e earliest-deadline-first (`edf`) — para tarefas periódicas críticas.
 
## Arquivos
 
- **main.c** — ponto de entrada do programa. Faz a validação dos argumentos de
  linha de comando, chama o parsing do arquivo de entrada e a simulação, e
  escreve o resultado no arquivo de saída.
- **funcoes.c** — implementação da lógica do simulador: leitura e validação do
  arquivo de entrada (`parse_arquivo`), escolha da tarefa a executar em cada
  instante segundo o algoritmo escolhido (`escolhe_tarefa`), registro dos
  blocos do log de execução (`fechar_bloco`) e o laço principal da simulação
  (`loop_principal`).
- **funcoes.h** — declarações das structs (`task`, `bloco_log`) e das funções
  implementadas em `funcoes.c`, incluídas por `main.c`.
- **Makefile** — compila os dois arquivos-fonte no executável `scheduler` e
  remove os arquivos gerados (alvo `clean`).
## Como compilar
 
```
make
```
 
Gera o executável `scheduler` na pasta atual.
 
## Como executar
 
```
./scheduler rate <arquivo_de_entrada>
./scheduler edf <arquivo_de_entrada>
```
 
O resultado é escrito em `rate_jrxs.out` ou `edf_jrxs.out` (conforme o
algoritmo escolhido); nada é impresso na saída padrão durante a execução
normal. Em caso de entrada inválida, uma mensagem de erro é escrita em
stderr e o programa termina com código de saída diferente de zero, sem
criar arquivo de saída.
 
## Como limpar os arquivos gerados
 
```
make clean
```
 
Remove o executável `scheduler` e os arquivos `.out` gerados por execuções
anteriores.
 
## Como foi testado
 
- O exemplo do próprio enunciado (`voo.txt`, tarefas ATT e NAV) foi rodado
  em ambos os algoritmos e o log de execução, as perdas de deadline, as
  conclusões e os "killed" foram conferidos linha a linha contra a saída
  esperada.
- Um cenário com utilização de CPU igual a 100% (duas tarefas com
  `C/P` somando 1.0) foi usado para comparar rate-monotonic e EDF: sob
  rate-monotonic a tarefa de maior período perde deadline, enquanto sob
  EDF nenhuma tarefa perde — essa comparação está detalhada no relatório.
- Os cinco casos de entrada inválida exigidos pela especificação foram
  testados manualmente: número incorreto de argumentos, algoritmo diferente
  de `rate`/`edf`, arquivo inexistente, arquivo malformado (campo faltando)
  e tarefa violando `C ≤ D ≤ P`. Em todos os casos foi confirmado que a
  mensagem de erro aparece em stderr, o código de saída é diferente de
  zero e nenhum arquivo `.out` é criado.
## Sistema operacional
 
Desenvolvido e testado em Windows com WSL (Ubuntu), usando `gcc`.