#include "swap.h"

void funcionSwap() {
    SolicitudSwap* solicitud;
    while (1) {
        sem_wait(&semSwap);
        solicitud = list_remove(listaSwap,0);
        switch (solicitud->codigo) {
            case NUEVO:
                crearSwap(solicitud->numeroTabla);
                break;
            case SUSPENDIDO:
                suspenderSwap(solicitud->numeroTabla, solicitud->listaEscribirSuspended);
                break;
            case FIN:
                eliminarSwap(solicitud->numeroTabla);
                break;
            case LEER_PAGINA_SWAP:
                leerPaginaSwap(solicitud->numeroTabla, solicitud->paginaLectura, solicitud->bufferSolicitud);
                break;
            case LEER_Y_ESCRIBIR_PAGINA_SWAP:
                leerEscribirPaginaSwap(solicitud->numeroTabla, solicitud->paginaLectura, solicitud->bufferSolicitud, solicitud->paginaEscritura);
                break;
            default:
                log_info(logger,"Solicitud desconocida");
        }
        destruirSolicitudSwap(solicitud);
        usleep(config.RETARDO_SWAP * 1000);
        sem_post(&semEsperarSwap);
    }
}

void enviarSolicitudSwap(MENSAJE_KERNEL codigo, uint32_t numeroTabla, t_list* listaSusp, void** buffer, int pagLectura, int pagEscritura) {
    SolicitudSwap* solicitud = malloc(sizeof(SolicitudSwap));
    solicitud->codigo = codigo;
    solicitud->numeroTabla = numeroTabla;
    solicitud->listaEscribirSuspended = list_create();
    if (codigo == SUSPENDIDO) {list_destroy(solicitud->listaEscribirSuspended); solicitud->listaEscribirSuspended=listaSusp;}
    if (codigo == LEER_PAGINA_SWAP) {
        solicitud->bufferSolicitud = buffer;
        solicitud->paginaLectura = pagLectura;
    }
    if (codigo == LEER_Y_ESCRIBIR_PAGINA_SWAP) { ////////
        solicitud->bufferSolicitud = buffer;
        solicitud->paginaLectura = pagLectura;
        solicitud->paginaEscritura = pagEscritura;
    }
    list_add(listaSwap,solicitud);
    sem_post(&semSwap);
}

void iniciarSwap() {
    if (pthread_create(&hiloSwap, NULL, (void*) &funcionSwap, NULL) != 0) {
        log_error(logger, "Error al crear el hilo del swap");
	} else log_info (logger, "Swap iniciado correctamente");
}

void crearSwap(uint32_t numeroTabla) {
    EstructurasProceso* est = list_get(listaTablas, numeroTabla);
    uint32_t id = est->id;
    char* ruta = obtenerRuta(id);
    verificar_dir(config.PATH_SWAP);
    est->archivo = open(ruta, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
    if (est->archivo != -1) {
        ftruncate(est->archivo, est->size);
        log_info(logger, "Archivo swap del proceso %d fue creado correctamente",id);
    } else log_info (logger, "No se pudo crear el archivo swap del proceso %d", id);
    free(ruta);
}

void verificar_dir(char* path){
    if(!existe_dir(path)){
        log_info(logger,"Carpeta %s no encontrada, creando",path);
        mkdir(path, 0777);
    }
    else
        log_info(logger,"Carpeta %s encontrada",path);
}

int existe_dir(char* path)
{
    DIR* dir = opendir(path);
    if (dir) {
        closedir(dir);
        return 1;
    } else {
        return 0;
    }
}

void eliminarSwap(uint32_t numeroTabla) {
    EstructurasProceso* est = list_get(listaTablas, numeroTabla);
    uint32_t id = est->id;
    char* ruta = obtenerRuta(id);
    int resultadoClose = close(est->archivo);
    int resultadoRemove = remove(ruta);
    if (resultadoClose || resultadoRemove) log_info(logger, "No se pudo eliminar el archivo swap del proceso %d", id);
    else log_info(logger, "Archivo swap del proceso %d fue eliminado correctamente",id);
    free(ruta);
}

void suspenderSwap(uint32_t numeroTabla, t_list* escrituras) {
    EstructurasProceso* est = list_get(listaTablas, numeroTabla);
    EscrituraSwap* escrituraSwap;
    int i;
    for (i=0; i<list_size(escrituras); i++) {
        escrituraSwap = list_get(escrituras,i);
        escribirSwap(est->archivo,escrituraSwap->pagina,escrituraSwap->buffer);
    }
    log_info(logger,"Paginas del proceso %d guardadas en swap correctamente tras suspensión",est->id);
}

void leerPaginaSwap(uint32_t numeroTabla, int pagina, void** bufferSolicitud) {
    EstructurasProceso* est = list_get(listaTablas, numeroTabla);
    free(*bufferSolicitud);
    *bufferSolicitud = leerSwap(est->archivo, pagina);
    log_info(logger,"Se leyó la pagina en swap");
}

void leerEscribirPaginaSwap(uint32_t numeroTabla, int paginaLectura, void** bufferSolicitud, int paginaEscritura) {
    EstructurasProceso* est = list_get(listaTablas, numeroTabla);
    escribirSwap(est->archivo,paginaEscritura,*bufferSolicitud);
    free(*bufferSolicitud);
    log_info(logger,"Se escribió la pagina reemplazada en el swap");
    usleep(config.RETARDO_SWAP * 1000);
    *bufferSolicitud = leerSwap(est->archivo, paginaLectura);
    log_info(logger,"Se leyó la pagina nueva en el swap");
}

void* leerSwap(int archivo, int pagina) {
    void* buf = malloc(config.TAM_PAGINA);
    lseek(archivo, config.TAM_PAGINA * pagina, SEEK_SET);
    read(archivo, buf, config.TAM_PAGINA);
    return buf;
}

void escribirSwap(int archivo, int pagina, void* buffer) {
    lseek(archivo, config.TAM_PAGINA * pagina, SEEK_SET);
    write(archivo, buffer, config.TAM_PAGINA);
}

char* obtenerRuta (uint32_t id) {
    char* ruta = string_new();
    char* numero = string_itoa(id);
	string_append(&ruta, config.PATH_SWAP);
	string_append(&ruta, "/");
    string_append(&ruta, numero);
    string_append(&ruta, ".swap");
    free(numero);
    return ruta;
}
