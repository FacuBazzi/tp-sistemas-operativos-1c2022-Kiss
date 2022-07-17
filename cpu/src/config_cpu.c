#include "config_cpu.h"

void iniciar_config(void){
    cargar_config(pathConfig);
    setear_no_op_restantes();
}

void cargar_config(char* path) {
    id_ultimo_PCB_usado = -1;
    config = malloc(sizeof(Config));
    auxConfig = config_create(path);
	if (auxConfig != NULL) {
		config->entradas_tlb             = config_get_int_value(auxConfig, "ENTRADAS_TLB");
		config->reemplazo_tlb            = config_get_string_value(auxConfig, "REEMPLAZO_TLB");
		config->retardo_noop             = config_get_int_value(auxConfig, "RETARDO_NOOP");
		config->ip_memoria               = config_get_string_value(auxConfig, "IP_MEMORIA");
		config->puerto_memoria           = config_get_string_value(auxConfig, "PUERTO_MEMORIA");
		config->puerto_escucha_dispatch  = config_get_string_value(auxConfig, "PUERTO_ESCUCHA_DISPATCH");
		config->puerto_escucha_interrupt = config_get_string_value(auxConfig, "PUERTO_ESCUCHA_INTERRUPT");

        // printf("Reemplazo TLB: %s\n", config->reemplazo_tlb);
        // printf("Retardo NOOP: %d\n", config->retardo_noop);
        // printf("IP memoria: %s\n", config->ip_memoria);
        // printf("Puerto memoria: %s\n", config->puerto_memoria);
        // printf("Puerto escucha dispatch: %s\n", config->puerto_escucha_dispatch);
        // printf("Puerto escucha interrupt: %s\n", config->puerto_escucha_interrupt);

        log_info(logger, "Archivo de configuracion cargado correctamente desde %s",path);
    }
    else log_info(logger, "No se pudo leer el archivo de configuracion desde %s", path);
}

void setear_no_op_restantes() {
    int i;
    for (i=0; i<100; i++) {no_op_restantes[i]=-1;}
}
