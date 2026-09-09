#ifndef FUNCOES_H
#define FUNCOES_H
#include <stdio.h>

typedef struct task{
    char nome[32];        
    int periodo;         
    int deadline_rel;     
    int burst;             
    int indice_arquivo;    
    int proxima_chegada;  
    int deadline_absoluto;  
    int restante;           
    int completadas;    
    int perdidas;         
    int killed;
}task;

typedef struct bloco_log {
    int tarefa;   
    char razao;  
    int duracao;
} bloco_log;

int transforma_string_numero(const char *s);
int parse_arquivo(const char *caminho, int *tempo_total, task **tarefas, int *n_tarefas);
int escolhe_tarefa(task *tarefas, int n, int eh_rate);
void fechar_bloco(bloco_log **log, int *n_log, int *cap_log, int tarefa_atual, char razao, int inicio, int fim);
void loop_principal(int tempo_total, task *tarefas, int n_tarefas, int eh_rate, bloco_log **log_out, int *n_log_out);
#endif