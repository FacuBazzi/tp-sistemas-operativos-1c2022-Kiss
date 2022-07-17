#include "consola.h"

uint8_t validar_argumentos_ingresados(int argumentos_total){
        logger = log_create("./cfg/consola.log", "CONSOLA", true, LOG_LEVEL_INFO);
        if(argumentos_total !=3){
                log_error(logger, "La cantidad de argumentos ingresados es incorrecta. \nEs necesario enviar: \n1- Tamaño del proceso \n2- Ruta de pseudocodigo de instruccions");
                return 0;
        } 
        return 1;
}

int main(int argc, char * argv[]){
        time_t tiempoInicio = time(NULL);
        if(!validar_argumentos_ingresados(argc)){
                log_destroy(logger);
                return 0;
        }
        uint32_t tamanioProceso = atoi(argv[1]);
        iniciar_programa(tamanioProceso, argv[2]); // no need to allocate
        conectar_con_kernel(tamanioProceso);
        time_t tiempoFin = time(NULL);
        int tiempoRafaga = (int)difftime(tiempoFin,tiempoInicio);
        log_info(logger,"La ejecución del proceso tardó %d segundos!",tiempoRafaga);
        finalizar_programa();
        return 0;
}

t_config_consola* iniciar_config(){
        t_config_consola* config = malloc(sizeof(t_config_consola));
        config->IP_KERNEL = NULL;
        config->PUERTO_KERNEL = NULL;
        return config; 
}

void destruir_instrucciones(t_list* lista_instrucciones){
        list_iterate(lista_instrucciones, (void*) liberar_instruccion);
}

void finalizar_programa(){        
        log_destroy(logger);
        liberar_config(config);

        if(instrucciones != NULL){
                free(instrucciones);        
                destruir_instrucciones(lista_instrucciones);
                list_destroy(lista_instrucciones);
        }
        
        close(socket_kernel);
}

void liberar_config(t_config_consola* config){
        free(config->IP_KERNEL);        
        free(config->PUERTO_KERNEL);
        free(config);
}                                                       

void iniciar_programa(int tamanio_proceso, char* ruta_instrucciones){
        config = iniciar_config();
        log_info(logger, "Consola iniciada");      

        if(!cargar_config(config, "config/consola.config")){
                log_error(logger, "No se pudo cargar la config");
        } else {
                log_info(logger, "PUERTO_KERNEL: %s",config->PUERTO_KERNEL);
                log_info(logger, "IP KERNEL: %s",config->IP_KERNEL);
        }

        log_warning(logger, "Tamaño del proceso: %d", tamanio_proceso);
        log_warning(logger, "Ruta de instrucciones: %s", ruta_instrucciones);
        instrucciones = leer_instrucciones(ruta_instrucciones);

        if(instrucciones == NULL){
                log_error(logger, "Las instrucciones no pudieron ser cargadas");
        } else {
        lista_instrucciones = cargar_instrucciones(instrucciones);
        }
}

void mostrar_lista_instrucciones(t_list* lista_instrucciones){
        log_info(logger, "Instrucciones del pseudocodigo parseadas correctamente. Se agregaron a una lista...\n----------------- Instrucciones parseadas -------------------");
        log_info(logger, "TOTAL INSTRUCCIONES LEIDAS: %d", list_size(lista_instrucciones));

        for(int i = 0; i < list_size(lista_instrucciones); i++){
                log_warning(logger, "ELEMENTO %d LISTA INSTRUCCIONES: ", i+1);
                log_info(logger, "Instruccion: %d",  ((t_instruccion*) (list_get(lista_instrucciones, i)))->identificador);
                log_info(logger, "Parametro1: %d",  ((t_instruccion*) (list_get(lista_instrucciones, i)))->primer_parametro);
                log_info(logger, "Parametro2: %d",  ((t_instruccion*) (list_get(lista_instrucciones, i)))->segundo_parametro);
        }
}

