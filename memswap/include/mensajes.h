#ifndef MENSAJES_H
#define MENSAJES_H

#include "estructuras.h"

typedef enum {
	NUEVO,
	SUSPENDIDO,
	FIN,
	LEER_Y_ESCRIBIR_PAGINA_SWAP,
	LEER_PAGINA_SWAP
} MENSAJE_KERNEL;

typedef enum {
	TABLA1_TO_TABLA2,
	TABLA2_TO_FRAME,
	LECTURA,
	ESCRITURA
} MENSAJE_CPU;

PCB* recibirPCB(void* buffer, int desplazamiento);
void enviarConfigCPU();

#endif