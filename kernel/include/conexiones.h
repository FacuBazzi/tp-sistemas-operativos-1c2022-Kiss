#ifndef CONEXIONES_H
#define CONEXIONES_H

#include "planificacion.h"

int iniciar_servidor(char* ip, char* puerto, int cant_conexiones);
int conectarAServidor (char* ip, char* puerto);
void conectarConMemoria();
void conectarConCPU();
void abrirServidorParaProcesos();
void nuevoProceso ();
void esperarProcesos ();

#endif