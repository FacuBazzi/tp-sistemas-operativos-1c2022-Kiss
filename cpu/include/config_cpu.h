#ifndef CONFIG_CPU_H
#define CONFIG_CPU_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <stdlib.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <commons/collections/list.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/node.h>
#include <commons/string.h>
#include <pthread.h>
#include <assert.h>
#include <semaphore.h>
#include <time.h>
#include <sys/socket.h>
#include "logger_cpu.h"

//DECLARACIONES
typedef struct {
    int entradas_tlb;
    char* reemplazo_tlb;
    int retardo_noop;
    char* ip_memoria;
    char* puerto_memoria;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_interrupt;
} Config;

Config* config;
t_config* auxConfig;
int no_op_restantes[100];
int id_ultimo_PCB_usado;
char* pathConfig;

//DECLARACIONES DE FUNCIONES
void iniciar_config(void);
void cargar_config(char*);
void setear_no_op_restantes();
void asignarArchivoConfig(int argc, char** argv);

#endif