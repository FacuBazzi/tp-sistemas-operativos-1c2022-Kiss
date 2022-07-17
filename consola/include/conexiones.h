#ifndef CONEXIONES_H
#define CONEXIONES_H

#include "consola.h"

void conectar_con_kernel();
void conectarAServidor (char* ip, char* puerto);
void enviar_proceso(int socket_kernel, uint32_t tamanioProceso);
void recibir_fin_de_proceso(int socket_kernel);

#endif
