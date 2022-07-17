#ifndef INSTRUCCION_H
#define INSTRUCCION_H

#include <stdint.h>
#include <stdio.h>
#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include "logs.h"
#include <unistd.h>
#include <ctype.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include "shared_utils.h"

typedef struct{
    IDENTIFICADOR_INSTRUCCION identificador;
    uint32_t primer_parametro;
    uint32_t segundo_parametro;
} t_instruccion;

// PROTOTIPOS
char* leer_instrucciones(char* ruta);
char* obtener_instrucciones_todas(char* ruta_instruccion);
t_list* cargar_instrucciones(char* instrucciones);
uint32_t leer_numero(char* linea_instruccion, int *posicion_cursor, int param);
t_instruccion* inicializar_instruccion(IDENTIFICADOR_INSTRUCCION identificador, uint32_t primer_parametro, uint32_t segundo_parametro);
void liberar_instruccion(t_instruccion* instruccion);

#endif
