#ifndef MEMORIA_H
#define MEMORIA_H

#include "swap.h"

uint32_t nuevoProceso(PCB* pcb);
bool finProceso(uint32_t numeroTabla);
bool suspenderProceso(uint32_t numeroTabla);
void crearTablas(EstructurasProceso* est, PCB* pcb);
void asignarFrames(EstructurasProceso* est);
void liberarFrames(EstructurasProceso* est);
int obtenerPrimerFrameLibre();
t_list* hacerListaParaEscribirEnSwap(EstructurasProceso* est);
int obtenerNumeroPagina (EstructurasProceso* est, uint32_t numeroTabla2, uint32_t numeroEntrada);
void resetearTablas2 (EstructurasProceso* est);
uint32_t tabla1ToTabla2(uint32_t numeroTabla1, uint32_t entrada);
uint32_t tabla2ToFrame(uint32_t numeroTabla2, uint32_t numeroEntrada);
uint32_t realizarLectura(uint32_t direccionFisica);
uint32_t realizarEscritura(uint32_t direccionFisica, uint32_t valor);
void cargarPaginaEnMemoria(uint32_t numeroTabla2, uint32_t numeroEntrada, EntradaTabla2* entrada);
EntradaTabla2* obtenerEntradaSegunDireccionFisica(uint32_t direccionFisica);
void cargarPaginaEnFrameNuevo (EstructurasProceso* est, int numeroPagina, EntradaTabla2* entrada);
void cargarPaginaReemplazando (EstructurasProceso* est, int numeroPagina, EntradaTabla2* entrada);
FramePagina* obtenerFramePaginaVictima (EstructurasProceso* est);
void setearEntrada (EntradaTabla2* entrada, int frame, int presencia, int uso, int modificado);

void* MEMORIA;
int iteradorClock[100];

#endif