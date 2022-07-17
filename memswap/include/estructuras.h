#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <stdlib.h>
#include <stdint.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <commons/collections/list.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
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
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

typedef struct {
    char* PUERTO_ESCUCHA;
	int TAM_MEMORIA;
	int TAM_PAGINA;
	int ENTRADAS_POR_TABLA;
	int RETARDO_MEMORIA;
	char* ALGORITMO_REEMPLAZO;
	int MARCOS_POR_PROCESO;
	int RETARDO_SWAP;
	char* PATH_SWAP;
} Config;
Config config;

typedef struct {
	uint32_t id;
	int archivo;
	int* tabla1;
	uint32_t size;
	int cantidadTablas2;
	int cantidadPaginas;
	uint32_t numeroTabla;
	t_list* listaFramePagina;
} EstructurasProceso;

typedef struct {
	int frame;
	int presencia;
	int uso;
	int modificado;
	int numeroTabla1;
} EntradaTabla2;

typedef struct {
	int frame;
	int pagina;
	EntradaTabla2* entradaTabla2;
} FramePagina;

t_log* logger;
int socketKernel, socketCPU, socketServer, socketGenerico;
int tengoCPU, tengoKernel, cantidadMarcos;
t_list* listaTablas;
t_list* listaTablas2;
int* marcosDePaginas;

pthread_t hiloKernel;
pthread_t hiloCPU;
pthread_t hiloParaComandos;
pthread_t hiloServer;
pthread_t hiloSwap;

#endif