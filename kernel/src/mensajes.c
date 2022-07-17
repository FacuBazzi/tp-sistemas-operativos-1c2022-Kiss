#include "mensajes.h"

int calcularSizePCB(PCB* pcb) {
    int size = 0;
    int sizeInstruccion = sizeof(IDENTIFICADOR_INSTRUCCION) + sizeof(uint32_t)*2;
    size+=sizeof(uint32_t)*4;
    size+=sizeof(double);
    size+=sizeInstruccion*list_size(pcb->instrucciones);
    size+=sizeof(int); // cantidad de instrucciones
    return size;
}

void enviarPCB(PCB* pcb) {   
    int i;
    int size = calcularSizePCB(pcb);
    void* buffer = malloc(size);
    Instruccion* instruccion;
    uint32_t id = pcb->id;
    uint32_t tamanio = pcb->size;
    uint32_t contador = pcb->contador;
    uint32_t tabla = pcb->tabla;
    double estimacion = pcb->estimacion;
    int cantidadInstrucciones = list_size(pcb->instrucciones);
    int desplazamiento=0;

    memcpy(buffer+desplazamiento,&id,sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);
    memcpy(buffer+desplazamiento,&tamanio,sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);
    memcpy(buffer+desplazamiento,&contador,sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);
    memcpy(buffer+desplazamiento,&tabla,sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);
    memcpy(buffer+desplazamiento,&estimacion,sizeof(double));
    desplazamiento+=sizeof(double);
    memcpy(buffer+desplazamiento,&cantidadInstrucciones,sizeof(int));
    desplazamiento+=sizeof(int);
    for(i=0;i<cantidadInstrucciones;i++){
        instruccion = list_get(pcb->instrucciones,i);
        IDENTIFICADOR_INSTRUCCION identificador = instruccion->identificador;
        uint32_t parametro1 = instruccion->parametro1;
        uint32_t parametro2 = instruccion->parametro2;
        memcpy(buffer+desplazamiento,&identificador,sizeof(IDENTIFICADOR_INSTRUCCION));
        desplazamiento+=sizeof(IDENTIFICADOR_INSTRUCCION);
        memcpy(buffer+desplazamiento,&parametro1,sizeof(uint32_t));
        desplazamiento+=sizeof(uint32_t);
        memcpy(buffer+desplazamiento,&parametro2,sizeof(uint32_t));
        desplazamiento+=sizeof(uint32_t);
    }

    send(socketDispatch,&size, sizeof(int), 0);
    send(socketDispatch,buffer, size, 0);
    free(buffer);
    log_info(logger,"Proceso %d enviado al CPU",pcb->id);
}

PCB* recibirPCB() {
    PCB* pcb = malloc(sizeof(PCB));
    int desplazamiento=0;
    int size, cantInstrucciones, i;
    uint32_t id, tamanio, contador, tabla;
    double estimacion;
    Instruccion* instruccion;
    IDENTIFICADOR_INSTRUCCION identificador;
    uint32_t parametro1;
    uint32_t parametro2;

    recv(socketDispatch, &size, sizeof(int), 0);
	void* buffer = malloc(size);
    recv(socketDispatch, buffer, size, 0);

    memcpy(&id,buffer+desplazamiento,sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);
    memcpy(&tamanio,buffer+desplazamiento,sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);
    memcpy(&contador,buffer+desplazamiento,sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);
    memcpy(&tabla,buffer+desplazamiento,sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);
    memcpy(&estimacion,buffer+desplazamiento,sizeof(double));
    desplazamiento+=sizeof(double);
    memcpy(&cantInstrucciones,buffer+desplazamiento,sizeof(int));
    desplazamiento+=sizeof(int);

    pcb->id=id;
    pcb->size=tamanio;
    pcb->contador=contador;
    pcb->tabla=tabla;
    pcb->estimacion=estimacion;
    pcb->instrucciones=list_create();

    for(i=0;i<cantInstrucciones;i++){
        instruccion = malloc(sizeof(Instruccion));
        memcpy(&identificador,buffer+desplazamiento,sizeof(IDENTIFICADOR_INSTRUCCION));
        desplazamiento+=sizeof(IDENTIFICADOR_INSTRUCCION);
        memcpy(&parametro1,buffer+desplazamiento,sizeof(uint32_t));
        desplazamiento+=sizeof(uint32_t);
        memcpy(&parametro2,buffer+desplazamiento,sizeof(uint32_t));
        desplazamiento+=sizeof(uint32_t);
        instruccion->identificador=identificador;
        instruccion->parametro1=parametro1;
        instruccion->parametro2=parametro2;
        list_add(pcb->instrucciones, instruccion);
    }

    log_info(logger,"Proceso %d recibido desde el CPU",pcb->id);
    free(buffer);
    return pcb;
}

PCB* recibirProcesoYGenerarPCB() { 
    PCB* pcb = malloc(sizeof(PCB));
    int size, i, cantInstrucciones;
    int desplazamiento=0;
    uint32_t tamanioProceso;
    Instruccion* instruccion;
    IDENTIFICADOR_INSTRUCCION identificador;
    uint32_t parametro1;
    uint32_t parametro2;

    recv(socketProceso[idProximoProceso], &size, sizeof(int), 0);
	void* buffer = malloc(size);
    recv(socketProceso[idProximoProceso], buffer, size, 0);

    memcpy(&tamanioProceso,buffer+desplazamiento,sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);
    memcpy(&cantInstrucciones,buffer+desplazamiento,sizeof(int));
    desplazamiento+=sizeof(int);

    pcb->id=idProximoProceso;
    idProximoProceso++;
    pcb->size=tamanioProceso;
    pcb->contador=0;
    pcb->tabla=0;
    pcb->estimacion=config.ESTIMACION_INICIAL;
    pcb->instrucciones=list_create();

    for(i=0;i<cantInstrucciones;i++){
        instruccion = malloc(sizeof(Instruccion));
        memcpy(&identificador,buffer+desplazamiento,sizeof(IDENTIFICADOR_INSTRUCCION));
        desplazamiento+=sizeof(IDENTIFICADOR_INSTRUCCION);
        memcpy(&parametro1,buffer+desplazamiento,sizeof(uint32_t));
        desplazamiento+=sizeof(uint32_t);
        memcpy(&parametro2,buffer+desplazamiento,sizeof(uint32_t));
        desplazamiento+=sizeof(uint32_t);
        instruccion->identificador=identificador;
        instruccion->parametro1=parametro1;
        instruccion->parametro2=parametro2;
        list_add(pcb->instrucciones, instruccion);
    }

    free(buffer);
    return pcb;
}

uint32_t nuevoProcesoParaMemoria (PCB* pcb) {   
    int i;
    int size = calcularSizePCB(pcb);
    size+=sizeof(MENSAJE_MEMORIA);
    void* buffer = malloc(size);
    Instruccion* instruccion;
    uint32_t id = pcb->id;
    uint32_t tamanio = pcb->size;
    uint32_t contador = pcb->contador;
    uint32_t tabla = pcb->tabla;
    double estimacion = pcb->estimacion;
    MENSAJE_MEMORIA codigo = NUEVO;
    int cantidadInstrucciones = list_size(pcb->instrucciones);
    int desplazamiento=0;

    memcpy(buffer+desplazamiento,&codigo,sizeof(MENSAJE_MEMORIA));
    desplazamiento+=sizeof(MENSAJE_MEMORIA);
    memcpy(buffer+desplazamiento,&id,sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);
    memcpy(buffer+desplazamiento,&tamanio,sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);
    memcpy(buffer+desplazamiento,&contador,sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);
    memcpy(buffer+desplazamiento,&tabla,sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);
    memcpy(buffer+desplazamiento,&estimacion,sizeof(double));
    desplazamiento+=sizeof(double);
    memcpy(buffer+desplazamiento,&cantidadInstrucciones,sizeof(int));
    desplazamiento+=sizeof(int);
    for(i=0;i<cantidadInstrucciones;i++){
        instruccion = list_get(pcb->instrucciones,i);
        IDENTIFICADOR_INSTRUCCION identificador = instruccion->identificador;
        uint32_t parametro1 = instruccion->parametro1;
        uint32_t parametro2 = instruccion->parametro2;
        memcpy(buffer+desplazamiento,&identificador,sizeof(IDENTIFICADOR_INSTRUCCION));
        desplazamiento+=sizeof(IDENTIFICADOR_INSTRUCCION);
        memcpy(buffer+desplazamiento,&parametro1,sizeof(uint32_t));
        desplazamiento+=sizeof(uint32_t);
        memcpy(buffer+desplazamiento,&parametro2,sizeof(uint32_t));
        desplazamiento+=sizeof(uint32_t);
    }

    send(socketMemoria,&size, sizeof(int), 0);
    send(socketMemoria,buffer, size, 0);
    free(buffer);

    log_info(logger,"Proceso %d enviado a memoria, esperando respuesta",pcb->id);
    uint32_t respuesta;
    recv(socketMemoria, &respuesta, sizeof(uint32_t), 0);
    return respuesta;
}

void finalizarProcesoEnConsola (uint32_t id) {
    int fin=0;
    send(socketProceso[id],&fin, sizeof(int), 0);
    log_info(logger,"Finalizacion del proceso %d enviada a la consola correspondiente",id);
    close(socketProceso[id]);
}

bool avisoParaMemoria (MENSAJE_MEMORIA codigo, uint32_t numeroTabla) {
    bool respuesta=0;
    int size = sizeof(MENSAJE_MEMORIA) + sizeof(uint32_t);
    void* buffer = malloc(size);
    int desplazamiento=0;

    memcpy(buffer+desplazamiento,&codigo,sizeof(MENSAJE_MEMORIA));
    desplazamiento+=sizeof(MENSAJE_MEMORIA);
    memcpy(buffer+desplazamiento,&numeroTabla,sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);

    send(socketMemoria,&size, sizeof(int), 0);
    send(socketMemoria,buffer, size, 0);
    free(buffer);

    // recv(socketMemoria, &respuesta, sizeof(bool), 0);
    return respuesta;
}

void enviarInterrupcion () {
    if(!strcmp(config.ALGORITMO_PLANIFICACION,"SRT")) {
        int interrupcion=0;
        send(socketInterrupt,&interrupcion, sizeof(int), 0);
        log_info(logger,"Interrupcion enviada, esperando PCB");
    }
}
