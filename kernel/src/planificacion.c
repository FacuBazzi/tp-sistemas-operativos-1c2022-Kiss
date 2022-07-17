#include "planificacion.h"

void iniciarPlanificacion() {
    int error=0;
    if (pthread_create(&hiloPlanificacionCortoPlazo, NULL, (void*) &planificadorCortoPlazo, NULL) != 0) {
		log_error(logger, "Error al crear el hilo del planificador de corto plazo");
        error++;
	}
    if (pthread_create(&hiloPlanificacionLargoPlazo, NULL, (void*) &planificadorLargoPlazo, NULL) != 0) {
		log_error(logger, "Error al crear el hilo del planificador");
        error++;
	}
    if (pthread_create(&hiloBloqueos, NULL, (void*) &resolverBloqueos, NULL) != 0) {
		log_error(logger, "Error al crear el hilo de bloqueos");
        error++;
	}
    if (pthread_create(&hiloSuspended, NULL, (void*) &resolverSuspended, NULL) != 0) {
		log_error(logger, "Error al crear el hilo de suspended");
        error++;
	}
	if(error==0) log_info(logger,"Planificadores iniciados");
}

void planificadorCortoPlazo() {
    PCB* pcb;
    if(!strcmp(config.ALGORITMO_PLANIFICACION,"FIFO")) {
        while (1) {
            sem_wait(&semHayQueEjecutar);
            sem_wait(&semCantidadProcesosEnReady);
            pcb = list_remove(listaReady,0);
            list_add(listaExec,pcb);
            log_info(logger,"Pongo a ejecutar al proceso %d",pcb->id);
            sem_post(&semEnviarPCB);
        }
    }
    else if(!strcmp(config.ALGORITMO_PLANIFICACION,"SRT")) {
        while (1) {
            sem_wait(&semHayQueEjecutar);
            sem_wait(&semCantidadProcesosEnReady);
            pcb = list_remove(listaReady, obtenerProcesoConMenorRafagaRestante(listaReady));
            list_add(listaExec,pcb);
            log_info(logger,"Pongo a ejecutar al proceso %d",pcb->id);
            sem_post(&semEnviarPCB);
        }
    }
    else log_error(logger,"Algoritmo de planificacion no reconocido");
}

void planificadorMedianoPlazo(PCB* pcb) {
    int posicion;
    bool respuesta=0;
    log_info(logger,"Empieza el contador para la suspensión del proceso %d",pcb->id);
    usleep(config.TIEMPO_MAXIMO_BLOQUEADO*1000);
    pthread_mutex_lock(&mutexImportante);   ////////
    posicion = posicionDeProcesoEnLista(listaBlocked,pcb->id);
    if(posicion>=0) {
        respuesta = avisoParaMemoria (SUSPENDIDO, pcb->tabla);     // COMENTAR SI NO TENGO MEMORIA
        if (!respuesta) {
            log_info(logger,"El proceso %d se suspende por exceso de tiempo bloqueado",pcb->id);
            log_info(logger,"Suspensión del proceso %d confirmada por memoria",pcb->id);
            suspensiones[pcb->id]++;
            list_remove(listaBlocked,posicion);
            list_add(listaSuspendedBlocked,pcb);
            sem_post(&semCantidadProcesosEnSuspendedBlocked);
            sem_post(&semMultiprogramacion);
        } else log_info(logger,"Suspensión del proceso %d DENEGADA por memoria",pcb->id);
    }
    pthread_mutex_unlock(&mutexImportante);   ////////
}

void planificadorLargoPlazo() {
    PCB* pcb;
    uint32_t numeroTabla = 15;
    while (1) {
        sem_wait(&semCantidadProcesosEnNew);
        sem_wait(&semPermitidoParaNew);
        sem_post(&semPermitidoParaNew);
        sem_wait(&semMultiprogramacion);
        if (list_size(listaSuspendedReady)==0) {
            pcb = list_get(listaNew,0);
            numeroTabla = nuevoProcesoParaMemoria(pcb);     // COMENTAR SI NO TENGO MEMORIA
            if (numeroTabla<9999) {
                log_info(logger,"Inicializacion del proceso %d confirmada por memoria",pcb->id);
                pcb->tabla=numeroTabla;
                pcb = list_remove(listaNew,0);
                list_add(listaReady,pcb);
                log_info(logger,"Paso a ready al proceso %d",pcb->id);
                sem_post(&semCantidadProcesosEnReady);
                enviarInterrupcion();        // COMENTAR SI NO TENGO INTERRUPT
            } else log_info(logger,"Inicializacion del proceso %d DENEGADA por memoria",pcb->id);
        } else {
            sem_post(&semCantidadProcesosEnNew);
            sem_post(&semMultiprogramacion);
        }
    }
}

void intercambioConDispatch () {
    bool respuestaExit=0;
    double tiempoRafaga;
    time_t tiempoSend;
    time_t tiempoRecv;
    while(1) {
        sem_wait(&semEnviarPCB);
        PCB* pcb = list_remove(listaExec,0);
        enviarPCB(pcb);
        tiempoSend=time(NULL);
        destruirPCB(pcb);
        pcb = recibirPCB();
        tiempoRecv=time(NULL);
        tiempoRafaga=difftime(tiempoRecv,tiempoSend)*1000;
        if(!strcmp(config.ALGORITMO_PLANIFICACION,"SRT"))
        log_info(logger,"La duración de la rafaga de CPU fue de %f milisegundos",tiempoRafaga);
        IDENTIFICADOR_INSTRUCCION proxInstruccion = obtenerIdentificadorProximaInstruccion(pcb);
        if (proxInstruccion==EXIT) {
            respuestaExit = avisoParaMemoria (FIN, pcb->tabla);        // COMENTAR SI NO TENGO MEMORIA
            if (!respuestaExit) {
                log_info(logger,"Finalizacion del proceso %d confirmada por memoria",pcb->id);
                finalizarProcesoEnConsola(pcb->id);
                list_add(listaExit,pcb);
                log_info(logger,"Mando a exit al proceso %d",pcb->id);
                sem_post(&semMultiprogramacion);
            } else log_info(logger,"Finalizacion del proceso %d DENEGADA por memoria",pcb->id);
        }
        else if (proxInstruccion==IO) {
            if(!strcmp(config.ALGORITMO_PLANIFICACION,"SRT")) actualizarEstimacion(pcb,tiempoRafaga);
            entradaSalida[pcb->id]++;
            list_add(listaBlocked,pcb);
            list_add(listaPendientesDeDesbloqueo,pcb);
            log_info(logger,"Pongo en bloqueado al proceso %d",pcb->id);
            pthread_create(&hiloPlanificacionMedianoPlazo[pcb->id], NULL, (void*) &planificadorMedianoPlazo, pcb);
            pthread_detach(hiloPlanificacionMedianoPlazo[pcb->id]);
            sem_post(&semCantidadProcesosEsperandoDesbloqueo);
        }
        else {
            if(!strcmp(config.ALGORITMO_PLANIFICACION,"SRT")) pcb->estimacion = pcb->estimacion - tiempoRafaga;
            list_add(listaReady,pcb);
            log_info(logger,"Pongo en ready al proceso %d",pcb->id);
            sem_post(&semCantidadProcesosEnReady);
        }
        sem_post(&semHayQueEjecutar);
    }
}

void resolverBloqueos() {
    PCB* pcb;
    Instruccion* ins;
    int tiempo, posicion;
    while(1) {
        sem_wait(&semCantidadProcesosEsperandoDesbloqueo);
        pcb = list_remove(listaPendientesDeDesbloqueo,0);
        ins = list_get(pcb->instrucciones,pcb->contador);
        tiempo = ins->parametro1;
        log_info(logger,"El proceso %d comienza a hacer su IO",pcb->id);
        usleep(tiempo*1000);
        pthread_mutex_lock(&mutexImportante);   ////////
        pcb->contador++;
        posicion = posicionDeProcesoEnLista (listaBlocked, pcb->id);
        if (posicion>=0) {
            list_remove(listaBlocked,posicion);
            list_add(listaReady,pcb);
            log_info(logger,"El proceso %d finaliza de hacer su IO y vuelve a ready",pcb->id);
            sem_post(&semCantidadProcesosEnReady);
            enviarInterrupcion();
        }
        else {
            posicion = posicionDeProcesoEnLista (listaSuspendedBlocked, pcb->id);
            if (posicion>=0) {
                list_remove(listaSuspendedBlocked,posicion);
                list_add(listaSuspendedReady,pcb);
                if (list_size(listaSuspendedReady)==1) sem_wait(&semPermitidoParaNew);
                log_info(logger,"El proceso %d finaliza de hacer su IO pero está suspendido",pcb->id);
                sem_wait(&semCantidadProcesosEnSuspendedBlocked);
                sem_post(&semCantidadProcesosEnSuspendedReady);
            } else log_error(logger,"Perdi al proceso %d",pcb->id);
        }
        pthread_mutex_unlock(&mutexImportante);   ////////
    }
}

void resolverSuspended() {
    PCB* pcb;
    while (1) {
        sem_wait(&semCantidadProcesosEnSuspendedReady);
        sem_wait(&semMultiprogramacion);
        pcb = list_remove(listaSuspendedReady,0);
        log_info(logger,"El proceso %d vuelve a ready luego de estar suspendido",pcb->id);
        list_add(listaReady,pcb);
        sem_post(&semCantidadProcesosEnReady);
        if (list_size(listaSuspendedReady)==0) sem_post(&semPermitidoParaNew);
        enviarInterrupcion();        // COMENTAR SI NO TENGO INTERRUPT
    }
}

IDENTIFICADOR_INSTRUCCION obtenerIdentificadorProximaInstruccion (PCB* pcb) {
    Instruccion* ins = list_get(pcb->instrucciones,pcb->contador);
    return ins->identificador;
}

int posicionDeProcesoEnLista (t_list* lista, uint32_t id) {
    int i;
    PCB* pcb;
    for (i=0; i<list_size(lista); i++) {
        pcb = list_get(lista,i);
        if (pcb->id==id) return i;
    }
    return -1;
}

int obtenerProcesoConMenorRafagaRestante(t_list* lista) {
    int i;
    double menorRafaga = 0;
    PCB* pcb;
    int menorProceso = 0;
    for (i=0; i<list_size(lista); i++) {
        pcb = list_get(lista,i);
        if (pcb->estimacion < menorRafaga || i==0) {
            menorRafaga = pcb->estimacion;
            menorProceso = i;
        }
    }
    PCB* menorpcb = list_get(lista,menorProceso);
    log_info(logger,"El proceso en ready con menor ráfaga restante es el proceso %d, con una estimacion de %f",menorpcb->id,menorpcb->estimacion);
    return menorProceso;
}

void actualizarEstimacion(PCB* pcb, double rafaga) {
    // Est(n+1) = 𝝰 R(n) + ( 1 - 𝝰 ) Est(n)
    double estimacionNueva = config.ALFA * rafaga + (1-config.ALFA) * pcb->estimacion;
    log_info(logger,"Actualizo la estimacion del proceso %d: pasa de %f a %f milisegundos",pcb->id,pcb->estimacion,estimacionNueva);
    pcb->estimacion = estimacionNueva;
}
