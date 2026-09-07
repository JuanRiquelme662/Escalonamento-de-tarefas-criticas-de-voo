#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct task{
    char nome[32];        
    int periodo;         
    int deadline_rel;     
    int burst;             
    int indice_arquivo;    
    // ---- campos que mudam durante a simulação ----
    int proxima_chegada;  
    int deadline_absoluto;  
    int restante;           
    int completadas;    
    int perdidas;         
    int killed;
}task;

typedef struct {
    int tarefa;   // indice da tarefa, ou -1 para idle
    char razao;   // 'F', 'H', 'L', ou 0 se for bloco idle
    int duracao;
} bloco_log;

int transforma_string_numero(const char *s) {
    char *fim;
    long v = strtol(s, &fim, 10);
    if (*fim != '\0' || v <= 0){
    return -1; //identifica se sobrou lixo depois do numero ou se ele e negativo
    }
    return (int)v;
}

int parse_arquivo(const char *caminho, int *tempo_total, task **tarefas, int *n_tarefas) {
    FILE *f = fopen(caminho, "r");
    if (!f) {
        fprintf(stderr, "erro: nao foi possivel abrir '%s'\n", caminho);
        return -1;
    }

    char linha[256];
    if (!fgets(linha, sizeof(linha), f)) {
        fprintf(stderr, "erro: arquivo vazio\n");
        fclose(f); return -1;
    }
    *tempo_total = transforma_string_numero(strtok(linha, "\n"));
    if (*tempo_total <= 0) {
        fprintf(stderr, "erro: tempo total invalido\n");
        fclose(f); return -1;
    }

    int cap = 4, n = 0;
    task *lista = malloc(cap * sizeof(task));

    char nome[32], c1[32], c2[32], c3[32];
    while (fgets(linha, sizeof(linha), f)) {
        if (sscanf(linha, "%31s %31s %31s %31s", nome, c1, c2, c3) != 4) {
            if (strspn(linha, " \t\r\n") == strlen(linha)) continue; // linha em branco, ignora
            fprintf(stderr, "erro: linha malformada: '%s'\n", linha);
            free(lista); fclose(f); return -1;
        }

        int periodo = transforma_string_numero(c1);
        int deadline = transforma_string_numero(c2);
        int burst = transforma_string_numero(c3);
        if (periodo < 0 || deadline < 0 || burst < 0 || !(burst <= deadline && deadline <= periodo)) {
            fprintf(stderr, "erro: tarefa '%s' invalida\n", nome);
            free(lista); fclose(f); return -1;
        }

        if (n == cap) { cap *= 2; lista = realloc(lista, cap * sizeof(task)); }

        strncpy(lista[n].nome, nome, 31);
        lista[n].periodo = periodo;
        lista[n].deadline_rel = deadline;
        lista[n].burst = burst;
        lista[n].indice_arquivo = n;
        lista[n].proxima_chegada = 0;
        lista[n].deadline_absoluto = 0;
        lista[n].restante = 0;
        lista[n].completadas = lista[n].perdidas = lista[n].killed = 0;
        n++;
    }
    fclose(f);

    if (n == 0) { fprintf(stderr, "erro: nenhuma tarefa\n"); free(lista); return -1; }

    *tarefas = lista;
    *n_tarefas = n;
    return 0;
}

int escolhe_tarefa(task *tarefas, int n, int eh_rate) {
    int escolhida = -1;
    for (int i = 0; i < n; i++) {
        if (tarefas[i].restante <= 0) continue;
        if (escolhida == -1) { escolhida = i; continue; }
        int pri_atual   = eh_rate ? tarefas[i].periodo         : tarefas[i].deadline_absoluto;
        int pri_melhor  = eh_rate ? tarefas[escolhida].periodo : tarefas[escolhida].deadline_absoluto;
        if (pri_atual < pri_melhor) escolhida = i;
    }
    return escolhida;
}

int loop_principal(int tempo_total, task *tarefas, int n_tarefas, int eh_rate) {
    for (int t = 0; t < tempo_total; t++) {

        // 1. verifica se alguma tarefa perdeu o deadline agora
        for (int i = 0; i < n_tarefas; i++) {
                if (tarefas[i].restante > 0 && tarefas[i].deadline_absoluto == t) {
                    tarefas[i].perdidas++;
                    tarefas[i].restante = 0;
                }
        }

            // 2. verifica se alguma tarefa chega agora (nova instancia)
        for (int i = 0; i < n_tarefas; i++) {
            if (tarefas[i].proxima_chegada == t) {
                tarefas[i].restante = tarefas[i].burst;
                tarefas[i].deadline_absoluto = t + tarefas[i].deadline_rel;
                tarefas[i].proxima_chegada += tarefas[i].periodo;
            }
        }

            // 3. escolhe quem roda neste instante
        int escolhida = escolhe_tarefa(tarefas, n_tarefas, eh_rate);

        // 4. executa 1 unidade de tempo da tarefa escolhida
        if (escolhida != -1) {
            tarefas[escolhida].restante--;
            if (tarefas[escolhida].restante == 0) {
                tarefas[escolhida].completadas++;
            }
        }
    }
    return 0;
}
int main(int argc, char *argv[]){
    if(argc != 3){
        fprintf(stderr, "formatacao esperada: ./scheduler rate/edf <arquivo de entrada>\n");
        return 1;
    }

    int eh_rate;
    if(strcmp(argv[1], "rate") == 0){
        eh_rate = 1;
    }else if(strcmp(argv[1], "edf") == 0){
        eh_rate = 0;
    }else{
        fprintf(stderr, "formatacao esperada: ./scheduler rate/edf <arquivo de entrada>\n");
        return 1;
    }

    int tempo_total;
    task *tarefas;
    int n_tarefas;

    if (parse_arquivo(argv[2], &tempo_total, &tarefas, &n_tarefas) != 0) {
        // parse_arquivo ja imprimiu o erro em stderr
        return 1;
    }

    // ---- debug temporario: conferir se os dados foram lidos certo ----
    printf("tempo total: %d\n", tempo_total);
    for (int i = 0; i < n_tarefas; i++) {
        printf("tarefa %d: nome=%s periodo=%d deadline=%d burst=%d\n",
               i, tarefas[i].nome, tarefas[i].periodo,
               tarefas[i].deadline_rel, tarefas[i].burst);
    }

    free(tarefas);
    return 0;
}