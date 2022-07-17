#ifndef CONEXIONES_H
#define CONEXIONES_H

#include "memoria.h"

int iniciar_servidor(char* ip, char* puerto, int cant_conexiones);
void abrirServidor();
void conectarConClientes();
void esperarMensajesCPU();
void esperarMensajesKernel();
void resolverHandshake();

#endif