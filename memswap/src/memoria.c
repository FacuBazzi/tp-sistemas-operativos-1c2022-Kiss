#include "memoria.h"

uint32_t nuevoProceso(PCB* pcb) {
    uint32_t numeroTabla = 5;
    EstructurasProceso* est = malloc(sizeof(EstructurasProceso));
    est->id = pcb->id;
    est->size = pcb->size;
    est->listaFramePagina = list_create();
    numeroTabla = list_size(listaTablas);
    est->numeroTabla = numeroTabla;
    crearTablas(est, pcb);
    list_add(listaTablas,est);
    enviarSolicitudSwap(NUEVO, numeroTabla, (t_list*)NULL, (void*)NULL, (int)NULL, (int)NULL);
    sem_wait(&semEsperarSwap);
    destruirPCB(pcb);
    return numeroTabla;
}

bool finProceso(uint32_t numeroTabla) {
    bool resultado = 0;
    EstructurasProceso* est = list_get(listaTablas,numeroTabla);
    liberarFrames(est);
    enviarSolicitudSwap(FIN, numeroTabla, (t_list*)NULL, (void*)NULL, (int)NULL, (int)NULL);
    sem_wait(&semEsperarSwap);
    return resultado;
}

bool suspenderProceso(uint32_t numeroTabla) {
    bool resultado = 0;
    EstructurasProceso* est = list_get(listaTablas,numeroTabla);
    t_list* lista = hacerListaParaEscribirEnSwap(est);
    enviarSolicitudSwap(SUSPENDIDO, numeroTabla, lista, (void*)NULL, (int)NULL, (int)NULL);
    liberarFrames(est);
    resetearTablas2(est);
    sem_wait(&semEsperarSwap);
    return resultado;
} 

t_list* hacerListaParaEscribirEnSwap(EstructurasProceso* est) {
    t_list* lista = list_create();
    int i;
    FramePagina* framePagina;
    EscrituraSwap* escrituraSwap;
    void* bufferAux;
    for (i=0; i<list_size(est->listaFramePagina); i++) {
        framePagina = list_get(est->listaFramePagina,i);
        escrituraSwap=malloc(sizeof(EscrituraSwap));
        escrituraSwap->pagina = framePagina->pagina;
        bufferAux = malloc(config.TAM_PAGINA);
        memcpy(bufferAux, MEMORIA + config.TAM_PAGINA * framePagina->frame, config.TAM_PAGINA);
        escrituraSwap->buffer = bufferAux;
        list_add(lista,escrituraSwap);
    }
    return lista;
}

void crearTablas(EstructurasProceso* est, PCB* pcb) {   // revisar esto
    int i;
    t_list* tabla2;
    double aux = (double)pcb->size/(double)config.TAM_PAGINA;
    est->cantidadPaginas = ceil(aux);
    aux = (double)est->cantidadPaginas/(double)config.ENTRADAS_POR_TABLA;
    est->cantidadTablas2 = ceil(aux);
    est->tabla1 = malloc(sizeof(int) * est->cantidadTablas2);
    for (i=0; i<est->cantidadTablas2; i++) {
        est->tabla1[i] = list_size(listaTablas2);
        tabla2 = list_create();
        list_add(listaTablas2,tabla2);
    }
    resetearTablas2(est);
}

void resetearTablas2 (EstructurasProceso* est) {    // revisar esto
    int i, j, aux;
    t_list* tabla2;
    EntradaTabla2* entrada;
    for (i=0; i<est->cantidadTablas2; i++) {
        aux = est->tabla1[i];
        tabla2 = list_get(listaTablas2,aux);
        if (list_size(tabla2)>0) list_clean_and_destroy_elements(tabla2,(void*)free);
        for (j=0; j<config.ENTRADAS_POR_TABLA; j++) {
            entrada = malloc(sizeof(EntradaTabla2));
            setearEntrada(entrada,-1,0,0,0);
            entrada->numeroTabla1=est->numeroTabla;
            list_add(tabla2,entrada);
        }
    }
}

void liberarFrames(EstructurasProceso* est) {
    int i, aux;
    FramePagina* framePagina;
    for (i=0; i<list_size(est->listaFramePagina); i++) {
        framePagina = list_get(est->listaFramePagina,i);
        aux = framePagina->frame;
        marcosDePaginas[aux] = -1;
        free(framePagina);
    }
    list_clean(est->listaFramePagina);
}

int obtenerPrimerFrameLibre() {
    int i;
    for (i=0; i<cantidadMarcos; i++) {
        if (marcosDePaginas[i] == -1) return i;
    }
    return -1;
}

EntradaTabla2* obtenerEntradaSegunDireccionFisica(uint32_t direccionFisica) {
    int numeroFrame = floor(direccionFisica/config.TAM_PAGINA);
    uint32_t numeroTabla1 = marcosDePaginas[numeroFrame];
    EstructurasProceso* est = list_get(listaTablas,numeroTabla1);
    FramePagina* framePagina;
    int i, pagina;
    for (i=0; i<list_size(est->listaFramePagina); i++) {
        framePagina = list_get(est->listaFramePagina,i);
        if (framePagina->frame==numeroFrame) pagina=framePagina->pagina;
    }
    int entradaTabla1=floor(pagina/config.ENTRADAS_POR_TABLA);
    int entradaTabla2= pagina - entradaTabla1 * config.ENTRADAS_POR_TABLA;
    t_list* tabla2 = list_get(listaTablas2,est->tabla1[entradaTabla1]);
    EntradaTabla2* entrada = list_get(tabla2,entradaTabla2);
    return entrada;
}

uint32_t realizarLectura(uint32_t direccionFisica) {
    uint32_t lectura;
    memcpy(&lectura,MEMORIA+direccionFisica,sizeof(uint32_t));
    EntradaTabla2* entrada = obtenerEntradaSegunDireccionFisica(direccionFisica);
    entrada->uso=1;
    return lectura;
}

uint32_t realizarEscritura(uint32_t direccionFisica, uint32_t valor) {
    uint32_t respuesta = 0;
    memcpy(MEMORIA+direccionFisica,&valor,sizeof(uint32_t));
    EntradaTabla2* entrada = obtenerEntradaSegunDireccionFisica(direccionFisica);
    entrada->uso=1;
    entrada->modificado=1;
    return respuesta;
}

uint32_t tabla1ToTabla2(uint32_t numeroTabla1, uint32_t entrada) {
    uint32_t numeroTabla2;
    EstructurasProceso* est = list_get(listaTablas,numeroTabla1);
    numeroTabla2 = est->tabla1[entrada];
    return numeroTabla2;
}

uint32_t tabla2ToFrame(uint32_t numeroTabla2, uint32_t numeroEntrada) {
    uint32_t frame = 0;
    t_list* tabla2 = list_get(listaTablas2,numeroTabla2);
    EntradaTabla2* entrada = list_get(tabla2,numeroEntrada);
    entrada->uso=1;
    if (entrada->presencia == 0) cargarPaginaEnMemoria(numeroTabla2, numeroEntrada, entrada);
    frame = entrada->frame;
    return frame;
}

void cargarPaginaEnMemoria(uint32_t numeroTabla2, uint32_t numeroEntrada, EntradaTabla2* entrada) {
    EstructurasProceso* est = list_get(listaTablas,entrada->numeroTabla1);
    int numeroPagina = obtenerNumeroPagina(est,numeroTabla2,numeroEntrada);
    if (list_size(est->listaFramePagina)<config.MARCOS_POR_PROCESO)
        cargarPaginaEnFrameNuevo(est, numeroPagina, entrada);
    else cargarPaginaReemplazando(est, numeroPagina, entrada);
}

int obtenerNumeroPagina (EstructurasProceso* est, uint32_t numeroTabla2, uint32_t numeroEntrada) {
    int numeroPagina, i, aux;
    for (i=0; i<est->cantidadTablas2; i++) {
        if (est->tabla1[i]==numeroTabla2) aux=i;
    }
    numeroPagina = aux * config.ENTRADAS_POR_TABLA + numeroEntrada;
    return numeroPagina;
}

void cargarPaginaEnFrameNuevo (EstructurasProceso* est, int numeroPagina, EntradaTabla2* entrada) {
    FramePagina* framePagina = malloc(sizeof(FramePagina));
    int frameLibre = obtenerPrimerFrameLibre();
    marcosDePaginas[frameLibre] = est->numeroTabla;
    framePagina->frame = frameLibre;
    framePagina->pagina = numeroPagina;
    framePagina->entradaTabla2 = entrada;
    list_add(est->listaFramePagina,framePagina);
    setearEntrada(entrada,frameLibre,1,1,0);
    void* buf = malloc(config.TAM_PAGINA);
    enviarSolicitudSwap(LEER_PAGINA_SWAP, est->numeroTabla, (t_list*)NULL, &buf, numeroPagina, (int)NULL);
    sem_wait(&semEsperarSwap);
    memcpy(MEMORIA + config.TAM_PAGINA * framePagina->frame, buf, config.TAM_PAGINA);
    free(buf);
}

void cargarPaginaReemplazando (EstructurasProceso* est, int numeroPagina, EntradaTabla2* entrada) {
    FramePagina* framePagina = obtenerFramePaginaVictima(est);
    EntradaTabla2* entradaVieja = framePagina->entradaTabla2;
    int frameViejo = framePagina->frame;
    int paginaVieja = framePagina->pagina;
    int modificadoViejo = entradaVieja->modificado;
    framePagina->pagina=numeroPagina;
    framePagina->entradaTabla2=entrada;
    setearEntrada(entradaVieja,-1,0,0,0);
    setearEntrada(entrada,frameViejo,1,1,0);
    void* buf = malloc(config.TAM_PAGINA);
    if (modificadoViejo==1) {
        memcpy(buf, MEMORIA + config.TAM_PAGINA * frameViejo, config.TAM_PAGINA);
        enviarSolicitudSwap(LEER_Y_ESCRIBIR_PAGINA_SWAP, est->numeroTabla, (t_list*)NULL, &buf, numeroPagina, paginaVieja);
    } else enviarSolicitudSwap(LEER_PAGINA_SWAP, est->numeroTabla, (t_list*)NULL, &buf, numeroPagina, (int)NULL);
    sem_wait(&semEsperarSwap);
    memcpy(MEMORIA + config.TAM_PAGINA * framePagina->frame, buf, config.TAM_PAGINA);
    free(buf);
}

FramePagina* obtenerFramePaginaVictima (EstructurasProceso* est) {
    int i;
    int flag=1;
    int vuelta=-1;
    FramePagina* framePagina;
    EntradaTabla2* entradaParaClock;
    if(!strcmp(config.ALGORITMO_REEMPLAZO,"CLOCK")) {
        for (i=iteradorClock[est->id]; flag>0; i++) {
            if (i==list_size(est->listaFramePagina)) i=0;
            framePagina = list_get(est->listaFramePagina,i);
            entradaParaClock = framePagina->entradaTabla2;
            if (entradaParaClock->uso == 0) {
                iteradorClock[est->id]=i;
                flag=0;
            }
            else entradaParaClock->uso = 0;
        }
    }
    else if(!strcmp(config.ALGORITMO_REEMPLAZO,"CLOCK-M")) {
        for (i=iteradorClock[est->id]; flag>0; i++) {
            if (i==list_size(est->listaFramePagina)) i=0;
            if (vuelta==-1) vuelta=0;
            else if (vuelta == 0 && i==iteradorClock[est->id]) vuelta=1;
            framePagina = list_get(est->listaFramePagina,i);
            entradaParaClock = framePagina->entradaTabla2;
            if (vuelta==0) {
                if (entradaParaClock->uso == 0 && entradaParaClock->modificado == 0) {
                    iteradorClock[est->id]=i;
                    flag=0;
                }
            } else if (vuelta==1) {
                if (entradaParaClock->uso == 0) {
                    iteradorClock[est->id]=i;
                    flag=0;
                }
                else entradaParaClock->uso = 0;
            }
        }
    }
    else log_error(logger,"Algoritmo de reemplazo desconocido");
    return framePagina;
}

void setearEntrada (EntradaTabla2* entrada, int frame, int presencia, int uso, int modificado) {
    entrada->frame=frame;
    entrada->presencia=presencia;
    entrada->uso=uso;
    entrada->modificado=modificado;
}
