#ifndef CONSOLA_H
#define CONSOLA_H
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <stdlib.h>
#include <commons/collections/list.h>
#include <readline/readline.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <sys/socket.h>
#include "config.h"
#include "instruccion.h"
#include "conexiones.h"
#include <time.h>

// VARIABLES GLOBALES
t_log* logger;
t_config_consola* config;
t_instruccion* instruccion;
char* instrucciones;
t_list* lista_instrucciones;

int socket_kernel;

// PROTOTIPOS
t_config_consola* iniciar_config();
void liberar_config(t_config_consola* config);
void iniciar_programa();
void finalizar_programa();
void mostrar_lista_instrucciones(t_list* lista_instrucciones);

#endif