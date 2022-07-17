#include "kernel.h"

int main(int argc, char ** argv){
        asignarArchivoConfig(argc, argv);
        iniciarTodo();
        abrirServidorParaProcesos();
        iniciarPlanificacion();
        conectarConMemoria();  // COMENTAR SI NO TENGO MEMORIA
        conectarConCPU();
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
        else string_append(&pathConfig, "./cfg/KERNEL.config");
}

void recibirComandos() {
        int fin=0;
        int i;
        char* lectura;
        PCB* pcb;
        while(!fin) {
                lectura=readline("");
                if(!strcmp(lectura,"fin")) fin=1;
                if(!strcmp(lectura,"imprimir")) {
                        printf("Procesos finalizados (en orden): ");
                        for (i=0; i<list_size(listaExit); i++) {
                                pcb=list_get(listaExit,i);
                                printf("%d ",pcb->id);
                        }
                        // printf("\nLista New: ");
                        // for (i=0; i<list_size(listaNew); i++) {
                        //         pcb=list_get(listaNew,i);
                        //         printf("%d ",pcb->id);
                        // }
                        // printf("\nLista Ready: ");
                        // for (i=0; i<list_size(listaReady); i++) {
                        //         pcb=list_get(listaReady,i);
                        //         printf("%d ",pcb->id);
                        // }
                        // printf("\nLista Exec: ");
                        // for (i=0; i<list_size(listaExec); i++) {
                        //         pcb=list_get(listaExec,i);
                        //         printf("%d ",pcb->id);
                        // }
                        // printf("\nLista Blocked: ");
                        // for (i=0; i<list_size(listaBlocked); i++) {
                        //         pcb=list_get(listaBlocked,i);
                        //         printf("%d ",pcb->id);
                        // }
                        // printf("\nLista SuspendedBlocked: ");
                        // for (i=0; i<list_size(listaSuspendedBlocked); i++) {
                        //         pcb=list_get(listaSuspendedBlocked,i);
                        //         printf("%d ",pcb->id);
                        // }
                        // printf("\nLista SuspendedReady: ");
                        // for (i=0; i<list_size(listaSuspendedReady); i++) {
                        //         pcb=list_get(listaSuspendedReady,i);
                        //         printf("%d ",pcb->id);
                        // }
                        int flag = 0;
                        printf("\nSuspensiones realizadas: ");
                        for (i=0; i<100; i++) {
                                if (suspensiones[i]>0) {printf("\n  El proceso %d se suspendió %d veces",i,suspensiones[i]); flag=1;}
                        }
                        if (!flag) printf("Ninguna");
                        else flag=0;
                        printf("\nI/O realizadas: ");
                        for (i=0; i<100; i++) {
                                if (entradaSalida[i]>0) {printf("\n  El proceso %d realizó %d I/O",i,entradaSalida[i]); flag=1;}
                        }
                        if (!flag) printf("Ninguna");
                        printf("\n");
                }
                free(lectura);
        }
        matarHilos();
}

void esperarFinalizacionHilos() {
        pthread_join(hiloServer,NULL);
        pthread_join(hiloDispatch,NULL);
        pthread_join(hiloPlanificacionCortoPlazo,NULL);
        pthread_join(hiloPlanificacionLargoPlazo,NULL);
        pthread_join(hiloBloqueos,NULL);
        pthread_join(hiloSuspended,NULL);
}

void iniciarTodo() {
	logger = log_create("./cfg/KERNEL.log", "KERNEL", true, LOG_LEVEL_INFO);
	log_info(logger,"Inicia el modulo Kernel haciendo uso de shared -> %s", mi_funcion_compartida());
        config = cargarConfig(pathConfig);
        pthread_create(&hiloParaComandos, NULL, (void*) &recibirComandos, NULL);
        pthread_detach(hiloParaComandos);
        idProximoProceso=0;
        listaNew = list_create();
        listaReady = list_create();
        listaExec = list_create();
        listaExit = list_create();
        listaBlocked = list_create();
        listaSuspendedReady = list_create();
        listaSuspendedBlocked = list_create();
        listaDeTodosMisProcesos = list_create();
        listaPendientesDeDesbloqueo = list_create();
        sem_init(&semCantidadProcesosEnNew,1,0);
        sem_init(&semCantidadProcesosEnReady,1,0);
        sem_init(&semHayQueEjecutar,1,1);
        sem_init(&semMultiprogramacion,1,config.GRADO_MULTIPROGRAMACION);
        sem_init(&semEnviarPCB,1,0);
        sem_init(&semCantidadProcesosEnSuspendedReady,1,0);
        sem_init(&semCantidadProcesosEnSuspendedBlocked,1,0);
        sem_init(&semCantidadProcesosEsperandoDesbloqueo,1,0);
        sem_init(&semPermitidoParaNew,1,1);
        pthread_mutex_init(&mutexImportante, NULL);
}

void matarHilos() {
        pthread_cancel(hiloServer);
        pthread_cancel(hiloDispatch);
        pthread_cancel(hiloPlanificacionCortoPlazo);
        pthread_cancel(hiloPlanificacionLargoPlazo);
        pthread_cancel(hiloBloqueos);
        pthread_cancel(hiloSuspended);
}

void finalizar() {
	log_info(logger,"Finalizando el modulo Kernel");
        // pthread_cancel(hiloParaComandos);   no hace falta al parecer
	log_destroy(logger);
	config_destroy(auxConfig);
	close(socketServer);
        close(socketMemoria);
        close(socketDispatch);
        close(socketInterrupt);
        pthread_mutex_destroy(&mutexImportante);
        sem_destroy(&semCantidadProcesosEnNew);
        destruirListasConSusElementos();
//	... Hacer los free/close/destroy
}

Config cargarConfig(char* path) {
	Config config;
	auxConfig = config_create(path);
	if (auxConfig != NULL) {
		config.IP_MEMORIA = config_get_string_value(auxConfig, "IP_MEMORIA");
		config.PUERTO_MEMORIA = config_get_string_value(auxConfig, "PUERTO_MEMORIA");
		config.IP_CPU = config_get_string_value(auxConfig, "IP_CPU");
		config.PUERTO_CPU_DISPATCH = config_get_string_value(auxConfig, "PUERTO_CPU_DISPATCH");
		config.PUERTO_CPU_INTERRUPT = config_get_string_value(auxConfig, "PUERTO_CPU_INTERRUPT");
		config.PUERTO_ESCUCHA = config_get_string_value(auxConfig, "PUERTO_ESCUCHA");
		config.ALGORITMO_PLANIFICACION = config_get_string_value(auxConfig, "ALGORITMO_PLANIFICACION");
		config.ESTIMACION_INICIAL = config_get_double_value(auxConfig, "ESTIMACION_INICIAL");
                config.ALFA = config_get_double_value(auxConfig, "ALFA");
                config.GRADO_MULTIPROGRAMACION = config_get_int_value(auxConfig, "GRADO_MULTIPROGRAMACION");
                config.TIEMPO_MAXIMO_BLOQUEADO = config_get_int_value(auxConfig, "TIEMPO_MAXIMO_BLOQUEADO");
		log_info(logger,"Configuración cargada correctamente desde %s",path);
	}
	else log_info(logger,"No se pudo cargar la configuración desde %s",path);
        int i;
        for (i=0; i<100; i++) {
                suspensiones[i]=0;
                entradaSalida[i]=0;
        }
	return config;
}
