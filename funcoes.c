#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoes.h"

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
    //agora tempo total trata /n e/r
    *tempo_total = transforma_string_numero(strtok(linha, "\n\r"));
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
        lista[n].nome[31] = '\0';
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

void fechar_bloco(bloco_log **log, int *n_log, int *cap_log, int tarefa_atual, char razao, int inicio, int fim) {
    if (fim <= inicio){
    return;
    }

    if (*n_log == *cap_log) {
    *cap_log = (*cap_log == 0) ? 8 : *cap_log * 2;
    *log = realloc(*log, *cap_log * sizeof(bloco_log));
    }
   
    (*log)[*n_log].tarefa = tarefa_atual;
    (*log)[*n_log].razao = razao;
    (*log)[*n_log].duracao = fim - inicio;
    (*n_log)++;
}

void loop_principal(int tempo_total, task *tarefas, int n_tarefas, int eh_rate, bloco_log **log_out, int *n_log_out) {
    int current = -1;
    int seg_start = 0;

    bloco_log *log = NULL;
    int n_log = 0, cap_log = 0;
    for (int t = 0; t < tempo_total; t++) {

        for (int i = 0; i < n_tarefas; i++) {
            if (tarefas[i].restante > 0 && tarefas[i].deadline_absoluto == t) {
                tarefas[i].perdidas++;
                if (current == i) {
                    fechar_bloco(&log, &n_log, &cap_log, current, 'L', seg_start, t);
                    current = -1;
                    seg_start = t;
                }
                tarefas[i].restante = 0;
            }
        }
        
        for (int i = 0; i < n_tarefas; i++) {
            if (tarefas[i].proxima_chegada == t) {
                tarefas[i].restante = tarefas[i].burst;
                tarefas[i].deadline_absoluto = t + tarefas[i].deadline_rel;
                tarefas[i].proxima_chegada += tarefas[i].periodo;
            }
        }

        int escolhida = escolhe_tarefa(tarefas, n_tarefas, eh_rate);
        if (escolhida != current) {
            //nao gosto de operador ternario, mas utilizei para enxugar o codigo
            char razao = (current != -1) ? 'H' : 0;
            fechar_bloco(&log, &n_log, &cap_log, current, razao, seg_start, t);
            current = escolhida;
            seg_start = t;
        }

        if (escolhida != -1) {
            tarefas[escolhida].restante--;
            if (tarefas[escolhida].restante == 0) {
                tarefas[escolhida].completadas++;
                fechar_bloco(&log, &n_log, &cap_log, current, 'F', seg_start, t + 1);
                current = -1;
                seg_start = t + 1;
            }
        }
    }
    fechar_bloco(&log, &n_log, &cap_log, current, 0, seg_start, tempo_total);

    for (int i = 0; i < n_tarefas; i++) {
        if (tarefas[i].restante > 0) {
            tarefas[i].killed++;
        }
    }  
            
    *log_out = log;
    *n_log_out = n_log;
    return;
}