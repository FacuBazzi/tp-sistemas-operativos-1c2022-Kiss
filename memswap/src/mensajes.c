#include "mensajes.h"

PCB* recibirPCB(void* buffer, int desplazamiento) {    // recibir PCB nuevo desde kernel
    PCB* pcb = malloc(sizeof(PCB));
    int cantInstrucciones, i;
    uint32_t id, tamanio, contador, tabla;
    double estimacion;
    Instruccion* instruccion;
    IDENTIFICADOR_INSTRUCCION identificador;
    uint32_t parametro1;
    uint32_t parametro2;

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

    return pcb;
}

void enviarConfigCPU () {
    int size = sizeof(uint32_t) * 2;
    send(socketCPU, &size, sizeof(uint32_t), 0);
    void* buf = malloc(size);
    uint32_t entradas = config.ENTRADAS_POR_TABLA;
    uint32_t tamPagina = config.TAM_PAGINA;
    memcpy(buf,&entradas,sizeof(uint32_t));
    memcpy(buf+sizeof(uint32_t),&tamPagina,sizeof(uint32_t));
    send(socketCPU, buf, size, 0);
    free(buf);
}
