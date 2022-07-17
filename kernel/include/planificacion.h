#ifndef PLANIFICACION_H
#define PLANIFICACION_H

#include "mensajes.h"

void iniciarPlanificacion();
void planificadorCortoPlazo();
void planificadorMedianoPlazo(PCB* pcb);
void planificadorLargoPlazo();
void intercambioConDispatch ();
void resolverBloqueos();
void resolverSuspended();
IDENTIFICADOR_INSTRUCCION obtenerIdentificadorProximaInstruccion (PCB* pcb);
int posicionDeProcesoEnLista (t_list* lista, uint32_t id);
int obtenerProcesoConMenorRafagaRestante(t_list* lista);
void actualizarEstimacion(PCB* pcb, double rafaga);

pthread_mutex_t mutexImportante;

#endif