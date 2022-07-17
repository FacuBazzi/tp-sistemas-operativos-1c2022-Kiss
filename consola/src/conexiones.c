#include "conexiones.h"

void conectar_con_kernel(uint32_t tamanioProceso) {
    conectarAServidor (config->IP_KERNEL, config->PUERTO_KERNEL);
    printf("---- \t ---- \t ---- \t ---- \t ---- \t ---- \t ---- \t ----\t ----\n");
    log_info(logger,"Conexion exitosa con kernel");
    enviar_proceso(socket_kernel, tamanioProceso);
    recibir_fin_de_proceso(socket_kernel);
}

void recibir_fin_de_proceso(int socket_kernel){
    int fin = 0;
    recv(socket_kernel,&fin, sizeof(int), 0);
    log_warning(logger, "Aviso recibido de Kernel: proceso finalizado");
}

void enviar_proceso(int socket_kernel, uint32_t tamanioProceso){ 
    int cantidad_de_instrucciones = list_size(lista_instrucciones); 
    int desplazamiento = 0;
    size_t size_lista_instrucciones = sizeof(IDENTIFICADOR_INSTRUCCION) * cantidad_de_instrucciones + sizeof(uint32_t) * cantidad_de_instrucciones + sizeof(uint32_t) * cantidad_de_instrucciones;
    size_t stream1 = sizeof(int); // tamaño del entero sizeTotal a enviar
    size_t stream2 = sizeof(uint32_t) + sizeof(int) + size_lista_instrucciones; // tamaño de: tamañoProceso cantInstrucciones [identificador param1 param2]
    size_t sizeTotal = stream1 + stream2;  // tamaño sizeTotal representa TODO el flujo de bytes a enviar
    void* stream = malloc(sizeTotal);

    memcpy(stream + desplazamiento, &sizeTotal, sizeof(int));
    desplazamiento += sizeof(int);

    memcpy(stream + desplazamiento, &tamanioProceso, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(stream + desplazamiento, &cantidad_de_instrucciones, sizeof(int));
    desplazamiento += sizeof(int);

    for(int i = 0 ; i < cantidad_de_instrucciones; i++){
        memcpy(stream + desplazamiento, &((t_instruccion*) (list_get(lista_instrucciones, i)))->identificador, sizeof(IDENTIFICADOR_INSTRUCCION));
        desplazamiento += sizeof(IDENTIFICADOR_INSTRUCCION);
        memcpy(stream + desplazamiento, &((t_instruccion*) (list_get(lista_instrucciones, i)))->primer_parametro, sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t);
        memcpy(stream + desplazamiento, &((t_instruccion*) (list_get(lista_instrucciones, i)))->segundo_parametro, sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t);
    }

    send(socket_kernel, stream, sizeTotal, 0);
    log_info(logger, "Tamaño del proceso y lista de instrucciones enviadas a Kernel");
    free(stream);

    // recv()

}

void conectarAServidor (char* ip, char* puerto) {
	struct addrinfo hints;
	struct addrinfo *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(ip, puerto, &hints, &server_info)){   // error
        log_error(logger, "ERROR EN GETADDRINFO");
        exit(-1);
    }

    socket_kernel = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    setsockopt(socket_kernel, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    setsockopt(socket_kernel, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

    if(connect(socket_kernel, server_info->ai_addr, server_info->ai_addrlen) == -1){
        if (puerto == config->PUERTO_KERNEL) log_error(logger, "FALLO LA CONEXION CON KERNEL...");
        else log_error(logger, "FALLO LA CONEXION CON ALGO DESCONOCIDO...");
        exit(-1);
    }

    freeaddrinfo(server_info);
}