#ifndef KERNEL_H
#define KERNEL_H

#include "conexiones.h"

t_config* auxConfig;
char* pathConfig;

Config cargarConfig(char* path);
void finalizar();
void iniciarTodo();
void esperarFinalizacionHilos();
void matarHilos();
void recibirComandos();
void asignarArchivoConfig(int argc, char** argv);

#endif