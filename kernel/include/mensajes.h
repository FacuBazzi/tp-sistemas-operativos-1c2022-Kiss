#ifndef MENSAJES_H
#define MENSAJES_H

#include "estructuras.h"

typedef enum {
	NUEVO,
	SUSPENDIDO,
	FIN
} MENSAJE_MEMORIA;

int calcularSizePCB(PCB* pcb);
void enviarPCB(PCB* pcb);
PCB* recibirProcesoYGenerarPCB();
PCB* recibirPCB();
uint32_t nuevoProcesoParaMemoria (PCB* pcb);
bool avisoParaMemoria (MENSAJE_MEMORIA codigo, uint32_t numeroTabla);
void finalizarProcesoEnConsola (uint32_t id);
void enviarInterrupcion();

void destruirListasConSusElementos();

int idProximoProceso;

#endif