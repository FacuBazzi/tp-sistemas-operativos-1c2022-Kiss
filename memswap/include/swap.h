#ifndef SWAP_H
#define SWAP_H

#include "mensajes.h"

typedef struct {
	MENSAJE_KERNEL codigo;
	uint32_t numeroTabla;
	t_list* listaEscribirSuspended;
	int paginaLectura;
	void** bufferSolicitud;
	int paginaEscritura;
} SolicitudSwap;

typedef struct {
	int pagina;
	void* buffer;
} EscrituraSwap;

void funcionSwap();
void iniciarSwap();
char* obtenerRuta(uint32_t id);
void crearSwap(uint32_t numeroTabla);
void eliminarSwap(uint32_t numeroTabla);
void suspenderSwap(uint32_t numeroTabla, t_list* escrituras);
void escribirSwap(int archivo, int pagina, void* buffer);
void leerPaginaSwap(uint32_t numeroTabla, int pagina, void** bufferSolicitud);
void leerEscribirPaginaSwap(uint32_t numeroTabla, int paginaLectura, void** bufferSolicitud, int paginaEscritura);
void* leerSwap(int archivo, int pagina);
void enviarSolicitudSwap(MENSAJE_KERNEL codigo, uint32_t numeroTabla, t_list* listaSusp, void** buffer, int pagLectura, int pagEscritura);
void destruirSolicitudSwap(SolicitudSwap* solicitud);
void destruirEscrituraSwap(EscrituraSwap* escritura);
int existe_dir(char*);
void verificar_dir(char*);

t_list* listaSwap;
sem_t semSwap;
sem_t semEsperarSwap;

#endif