#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>

#include "shared_estructuras.h"

void destruirPCB(PCB*);
void destruirInstruccion(Instruccion*);
char* mi_funcion_compartida();

#endif