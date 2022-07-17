#ifndef CONEXIONES_CPU_H
#define CONEXIONES_CPU_H

#include <pthread.h>
#include "cpu.h"

//DECLARACIONES
int socket_servidor_dispatch, socket_servidor_interrupt, socket_pcb, socket_interrupcion;
int socket_cliente_dispatch, socket_cliente_interrupt, socket_cpu_memoria;

pthread_t hiloServerDispatch;
pthread_t hiloServerInterrupt;
pthread_t hiloClienteMemoria;
pthread_t hiloParaComandos;

typedef enum {
    TABLA1_TO_TABLA2,
    TABLA2_TO_FRAME,
    LECTURA,
    ESCRITURA
} MENSAJE_PARA_MEMORIA;

//DECLARACIONES DE FUNCIONES
void iniciar_conexiones(void);
void iniciar_servidores(void);
int crear_servidor(char*, const char*, char*);
int aceptar_cliente(int);
void iniciar_cliente_memoria(void);
void esperar_pcbs(void*);
void esperar_interrupciones(void*);
int crear_cliente(char *, char*);
void avisar_interrupcion(void);
uint32_t solicitar_direcciones_memoria(uint32_t tabla_obtenida, uint32_t entrada_obtenida, int cod_mensaje );
void iniciar_hilo_comandos();
void matarHilos();
void recibirComandos();

#endif