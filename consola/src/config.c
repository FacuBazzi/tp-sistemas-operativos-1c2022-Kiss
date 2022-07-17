#include "config.h"

extern t_log* logger;

uint8_t cargar_config(t_config_consola* config, char* ruta_config){
    t_config* cfg = config_create(ruta_config);

    if(cfg == NULL){
        log_error(logger, "No hay un archivo de configuracion en la ruta especificada");
        return 0;
    }

    config->IP_KERNEL = strdup(config_get_string_value(cfg, "IP_KERNEL"));
    config->PUERTO_KERNEL = strdup(config_get_string_value(cfg, "PUERTO_KERNEL"));

    config_destroy(cfg);
    return 1;
}