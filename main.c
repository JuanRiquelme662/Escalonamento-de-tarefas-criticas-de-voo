#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoes.h"

#define LOGIN "jrxs"

int main(int argc, char *argv[]){
    bloco_log *log = NULL;
    int n_log = 0;

    if(argc != 3){
        fprintf(stderr, "formatacao esperada: ./scheduler rate/edf <arquivo de entrada>\n");
        return 1;
    }

    int eh_rate;
    const char *nome_algo;
    if(strcmp(argv[1], "rate") == 0){
        eh_rate = 1;
        nome_algo = "rate";
    }else if(strcmp(argv[1], "edf") == 0){
        eh_rate = 0;
        nome_algo = "edf";
    }else{
        fprintf(stderr, "formatacao esperada: ./scheduler rate/edf <arquivo de entrada>\n");
        return 1;
    }

    int tempo_total;
    task *tarefas;
    int n_tarefas;

    if (parse_arquivo(argv[2], &tempo_total, &tarefas, &n_tarefas) != 0) {
        return 1;
    }

    loop_principal(tempo_total, tarefas, n_tarefas, eh_rate, &log, &n_log);

    char nome_saida[64];
    snprintf(nome_saida, sizeof(nome_saida), "%s_%s.out", nome_algo, LOGIN);

    FILE *out = fopen(nome_saida, "w");
    if (!out) {
        fprintf(stderr, "erro: nao foi possivel criar arquivo de saida '%s'\n", nome_saida);
        free(tarefas);
        free(log);
        return 1;
    }
    fprintf(out, "EXECUTION BY %s\n\n", eh_rate ? "RATE" : "EDF");

    for (int i = 0; i < n_log; i++) {
        if (log[i].tarefa == -1) {
            fprintf(out, "idle for %d units\n", log[i].duracao);
        } else {
            fprintf(out, "[%s] for %d units", tarefas[log[i].tarefa].nome, log[i].duracao);
            if (log[i].razao != 0) {
                fprintf(out, " - %c", log[i].razao);
            }
            fprintf(out, "\n");
        }
    }

    fprintf(out, "\nLOST DEADLINES\n");
        for (int i = 0; i < n_tarefas; i++) {
            fprintf(out, "[%s] %d\n", tarefas[i].nome, tarefas[i].perdidas);
        }

    fprintf(out, "\nCOMPLETE EXECUTION\n");
        for (int i = 0; i < n_tarefas; i++) {
            fprintf(out, "[%s] %d\n", tarefas[i].nome, tarefas[i].completadas);
        }

    fprintf(out, "\nKILLED\n");
        for (int i = 0; i < n_tarefas; i++) {
            fprintf(out, "[%s] %d\n", tarefas[i].nome, tarefas[i].killed);
        }

    fclose(out);
    free(log);
    free(tarefas);
    return 0;
}