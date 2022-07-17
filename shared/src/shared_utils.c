#include "shared_utils.h"

char* mi_funcion_compartida(){
    return "Hice uso de la shared!";
}

void destruirPCB(PCB* pcb) {
    list_destroy_and_destroy_elements(pcb->instrucciones,(void*)destruirInstruccion);
    free(pcb);
}

void destruirInstruccion(Instruccion* ins) {
    free(ins);
}