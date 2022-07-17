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

typedef struct {
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	char* IP_CPU;
	char* PUERTO_CPU_DISPATCH;
    char* PUERTO_CPU_INTERRUPT;
    char* PUERTO_ESCUCHA;
	char* ALGORITMO_PLANIFICACION;
	double ESTIMACION_INICIAL;
	double ALFA;
	int GRADO_MULTIPROGRAMACION;
	int TIEMPO_MAXIMO_BLOQUEADO;
} Config;

Config config;
t_log* logger;
int socketServer, socketMemoria, socketDispatch, socketInterrupt;
int socketProceso[1000];
int suspensiones[100];
int entradaSalida[100];

pthread_t hiloServer;
pthread_t hiloDispatch;
pthread_t hiloParaComandos;
pthread_t hiloPlanificacionCortoPlazo;
pthread_t hiloPlanificacionMedianoPlazo[1000];
pthread_t hiloPlanificacionLargoPlazo;
pthread_t hiloBloqueos;
pthread_t hiloSuspended;

t_list* listaNew;
t_list* listaReady;
t_list* listaExec;
t_list* listaExit;
t_list* listaBlocked;
t_list* listaSuspendedReady;
t_list* listaSuspendedBlocked;
t_list* listaDeTodosMisProcesos;
t_list* listaPendientesDeDesbloqueo;

sem_t semMultiprogramacion;
sem_t semCantidadProcesosEnNew;
sem_t semCantidadProcesosEnReady;
sem_t semCantidadProcesosEnSuspendedReady;
sem_t semCantidadProcesosEnSuspendedBlocked;
sem_t semCantidadProcesosEsperandoDesbloqueo;
sem_t semHayQueEjecutar;
sem_t semEnviarPCB;
sem_t semPermitidoParaNew;

#endif