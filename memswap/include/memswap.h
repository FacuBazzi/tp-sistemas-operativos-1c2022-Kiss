#ifndef MEMSWAP_H
#define MEMSWAP_H

#include "conexiones.h"

t_config* auxConfig;
char* pathConfig;

Config cargarConfig(char* path);
void finalizar();
void iniciarTodo();
void esperarFinalizacionHilos();
void matarHilos();
void recibirComandos();
void iniciarVariablesMemoria();
void destruirListasConSusElementos();
void destruirTabla2(t_list* tabla2);
void destruirEstructuras(EstructurasProceso* est);
void asignarArchivoConfig(int argc, char** argv);

#endif