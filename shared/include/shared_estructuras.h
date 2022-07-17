#ifndef SHARED_ESTRUCTURAS_H
#define SHARED_ESTRUCTURAS_H

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
#include <pthread.h>
#include <assert.h>
#include <semaphore.h>
#include <time.h>
#include <sys/socket.h>

typedef enum {
	NO_OP,
	IO,
	READ,
	COPY,
	WRITE,
	EXIT,
} IDENTIFICADOR_INSTRUCCION;

typedef struct {
	uint32_t id;
	uint32_t size;
	uint32_t contador;
	uint32_t tabla;
	double estimacion;
	t_list* instrucciones;
} PCB;

typedef struct {
	IDENTIFICADOR_INSTRUCCION identificador;
	uint32_t parametro1;
	uint32_t parametro2;
} Instruccion;

#endif