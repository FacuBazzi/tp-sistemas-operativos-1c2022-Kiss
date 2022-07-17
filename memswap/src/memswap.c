#include "memswap.h"

int main(int argc, char ** argv){
        asignarArchivoConfig(argc, argv);
        iniciarTodo();
        iniciarSwap();
        abrirServidor();
        esperarFinalizacionHilos();
        finalizar();
        return 0;
}

void asignarArchivoConfig(int cantidadArgumentos, char** argumentos) {
        pathConfig = string_new();
        if (cantidadArgumentos > 1) {
                string_append(&pathConfig, "./cfg/");
                string_append(&pathConfig, argumentos[1]);
                string_append(&pathConfig, ".config");
        }
        else string_append(&pathConfig, "./cfg/MEMSWAP.config");
}

void recibirComandos() {    // para que el modulo lea comandos
        int fin=0;
        char* lectura;
        while(!fin) {
                lectura=readline("");
                if(!strcmp(lectura,"fin")) fin=1;     // le pueden agregar mas comandos para testear o manejar el modulo
                free(lectura);
        }
        matarHilos();
}

void esperarFinalizacionHilos() {    // espera q todos los hilos finalicen para terminar
        pthread_join(hiloServer,NULL);
        pthread_join(hiloSwap,NULL);
        pthread_join(hiloKernel,NULL);
        pthread_join(hiloCPU,NULL);
}

void iniciarTodo() {   // todo lo que debe hacerse al iniciar el modulo
	logger = log_create("./cfg/MEMSWAP.log", "MEMSWAP", true, LOG_LEVEL_INFO);
	log_info(logger,"Inicia el modulo Memswap haciendo uso de shared -> %s", mi_funcion_compartida());
	config = cargarConfig(pathConfig);
        tengoCPU=0;
        tengoKernel=0;
        pthread_create(&hiloParaComandos, NULL, (void*) &recibirComandos, NULL);
        pthread_detach(hiloParaComandos);
        listaSwap = list_create();
        listaTablas = list_create();
        listaTablas2 = list_create();
        sem_init(&semSwap,1,0);
        sem_init(&semEsperarSwap,1,0);
        iniciarVariablesMemoria();
}

void matarHilos() {        // finaliza a los hilos en caso de querer terminar la ejecucion para que siga el camino feliz
        pthread_cancel(hiloKernel);
        pthread_cancel(hiloCPU);
        pthread_cancel(hiloServer);
        pthread_cancel(hiloSwap);
}

void finalizar() {      // todo lo que debe hacerse al finalizar el modulo
	log_info(logger,"Finalizando el modulo Memswap");
	log_destroy(logger);
	config_destroy(auxConfig);
	close(socketKernel);
        close(socketCPU);
        close(socketServer);
        close(socketGenerico);
        free(MEMORIA);
        destruirListasConSusElementos();
//	... Hacer los free/close/destroy
}

Config cargarConfig(char* path) {    // cargar configuracion
	Config config;
	auxConfig = config_create(path);
	if (auxConfig != NULL) {
		config.PUERTO_ESCUCHA = config_get_string_value(auxConfig, "PUERTO_ESCUCHA");
		config.TAM_MEMORIA = config_get_int_value(auxConfig, "TAM_MEMORIA");
		config.TAM_PAGINA = config_get_int_value(auxConfig, "TAM_PAGINA");
                config.ENTRADAS_POR_TABLA = config_get_int_value(auxConfig, "ENTRADAS_POR_TABLA");
                config.RETARDO_MEMORIA = config_get_int_value(auxConfig, "RETARDO_MEMORIA");
                config.ALGORITMO_REEMPLAZO = config_get_string_value(auxConfig, "ALGORITMO_REEMPLAZO");
                config.MARCOS_POR_PROCESO = config_get_int_value(auxConfig, "MARCOS_POR_PROCESO");
                config.RETARDO_SWAP = config_get_int_value(auxConfig, "RETARDO_SWAP");
                config.PATH_SWAP = config_get_string_value(auxConfig, "PATH_SWAP");
		log_info(logger,"Configuración cargada correctamente desde %s",path);
	}
	else log_info(logger,"No se pudo cargar la configuración desde %s",path);
	return config;
}

void iniciarVariablesMemoria() {
        MEMORIA = malloc(config.TAM_MEMORIA);
        cantidadMarcos = config.TAM_MEMORIA/config.TAM_PAGINA;
        marcosDePaginas = malloc(sizeof(int) * cantidadMarcos);
        int i;
        for (i=0; i<cantidadMarcos; i++) {
                marcosDePaginas[i]=-1;
        }
}

void destruirListasConSusElementos() {
        list_destroy_and_destroy_elements(listaTablas,(void*)destruirEstructuras);
        list_destroy_and_destroy_elements(listaTablas2,(void*)destruirTabla2);
        list_destroy_and_destroy_elements(listaSwap,(void*)destruirSolicitudSwap);
}

void destruirEstructuras(EstructurasProceso* est) {
        list_destroy_and_destroy_elements(est->listaFramePagina,(void*)free);
        free(est->tabla1);
        free(est);
}

void destruirTabla2(t_list* tabla2) {
        list_destroy_and_destroy_elements(tabla2,(void*)free);
}

void destruirSolicitudSwap(SolicitudSwap* solicitud) {
        list_destroy_and_destroy_elements(solicitud->listaEscribirSuspended,(void*)destruirEscrituraSwap);
        free(solicitud);
}

void destruirEscrituraSwap(EscrituraSwap* escritura) {
        free(escritura->buffer);
        free(escritura);
}
