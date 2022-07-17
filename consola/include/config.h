#ifndef CONFIG_H
#define CONFIG_H

#include <readline/readline.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include "logs.h"
#include <stdint.h>

typedef struct {
    char* IP_KERNEL;
    char* PUERTO_KERNEL;
} t_config_consola;

// PROTOTIPOS DE FUNCIONES
uint8_t cargar_config(t_config_consola* config, char* ruta_config);

#endif
