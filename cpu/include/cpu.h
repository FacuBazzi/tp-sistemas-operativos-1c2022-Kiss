#ifndef CPU_H
#define CPU_H

#include "config_cpu.h"
#include "conexiones_cpu.h"
#include "logger_cpu.h"
#include "mensajes_cpu.h"
#include "shared_estructuras.h"
#include "shared_utils.h"
#include <commons/collections/list.h>
#include <time.h>
#include <math.h>

//DECLARACIONES
typedef struct {
    uint32_t cant_entradas_por_tabla_de_paginas;
    uint32_t tamanio_pagina;
} mem_config;

typedef struct {
    int numero_pagina;
    int marco;
    time_t tstamp_creado;
    time_t tstamp_ultima_vez_usado;
} TLB;

t_list* lista_tlb;
mem_config* config_memoria;
t_config* auxConfig;

int check_exit, check_io, existe_interrupt;

pthread_mutex_t interrupt_mutex;

//DECLARACIONES DE FUNCIONES
void iniciar_cpu(void);
void intercambio_con_kernel(void);
void exec_ciclo_instruccion(PCB *);
int chequear_interrupciones();
void finalizar(void);
Instruccion* fetch(PCB *);
int decode(Instruccion* prox_instruccion);
uint32_t fetch_operands(PCB* pcb, Instruccion* prox_instruccion);
void execute(PCB *pcb, Instruccion* prox_instruccion, uint32_t valorLeidoParaCopy);
void intercambio_con_memoria(void);
void recibir_config_memoria();
void iniciar_TLB(void);
void limpiar_TLB(void);
void actualizar_TLB(TLB*);
int verificar_reemplazo_TLB(void);
void reemplazar_TLB_FIFO(TLB*);
void reemplazar_TLB_LRU(TLB*);
uint32_t traducir_direccion(uint32_t);
TLB* buscar_en_TLB(uint32_t);
uint32_t calcular_desplazamiento(uint32_t);
void esperar_finalizacion_hilos();
uint32_t obtener_direccion_fisica(uint32_t tabla_1, uint32_t dir_logica );
uint32_t enviar_operacion_a_memoria(int operacion_principal, uint32_t dir_fisica, Instruccion* prox_instruccion);
uint32_t traducir_direccion_recibida(uint32_t tamanio_pagina, uint32_t nro_frame, uint32_t desplazamiento);
uint32_t calcular_entrada1(uint32_t dir_logica);
uint32_t calcular_entrada2(uint32_t dir_logica);
uint32_t calcular_desplazamiento(uint32_t dir_logica);
// void actualizar_tstamp_pagina(TLB*);
int existe_entrada_con_marco(TLB* registro_tlb_nuevo);

#endif