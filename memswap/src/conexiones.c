#include "conexiones.h"

void abrirServidor() {
    socketServer = iniciar_servidor(NULL, config.PUERTO_ESCUCHA, 2); // NULL = "127.0.0.1"
    log_info(logger,"Servidor iniciado y esperando que lleguen clientes");
    if (pthread_create(&hiloServer, NULL, (void*) &conectarConClientes, NULL) != 0) {
        log_error(logger, "Error al crear el hilo del server");
	}
}

void conectarConClientes() {
    socketGenerico = accept(socketServer, NULL, NULL);
    resolverHandshake();
    socketGenerico = accept(socketServer, NULL, NULL);
    resolverHandshake();
}

void resolverHandshake() {
    int handshake;
    recv(socketGenerico, &handshake, sizeof(int), 0);
    if (handshake == 0  && !tengoCPU) {  // cpu
        socketCPU = socketGenerico;
        tengoCPU=1;
        send(socketCPU, &handshake, sizeof(uint32_t), 0);
        enviarConfigCPU();
        if (pthread_create(&hiloCPU, NULL, (void*) &esperarMensajesCPU, NULL) != 0) {
		    log_error(logger, "Error al crear el hilo con CPU");
	    }
        else log_info(logger,"CPU conectado, esperando mensajes");
    } else if (handshake == 1 && !tengoKernel) {  // kernel
        socketKernel = socketGenerico;
        tengoKernel=1;
        send(socketKernel, &handshake, sizeof(int), 0);
        if (pthread_create(&hiloKernel, NULL, (void*) &esperarMensajesKernel, NULL) != 0) {
		    log_error(logger, "Error al crear el hilo con Kernel");
	    }
        else log_info(logger,"Kernel conectado, esperando mensajes");
    } else log_error(logger, "Codigo de handshake no reconocido");
}

void esperarMensajesCPU() {
    void* buffer;
    uint32_t numeroTabla1, numeroTabla2, frame, entrada, lectura, direccionFisica, valor, resultado;
    int size, desplazamiento;
    MENSAJE_CPU mensaje;
	while(1) {
        desplazamiento=sizeof(MENSAJE_CPU);
        recv(socketCPU, &size, sizeof(int), 0);
        buffer = malloc(size);
        recv(socketCPU, buffer, size, 0);
        memcpy(&mensaje,buffer,sizeof(MENSAJE_CPU));
        switch(mensaje) {
            case TABLA1_TO_TABLA2:    // size y luego codigo,numeroTabla1(uint32_t),entrada(uint32_t)
                log_info(logger,"El CPU me solicita un numero de tabla de 2do nivel");
                memcpy(&numeroTabla1,buffer+desplazamiento,sizeof(uint32_t));
                desplazamiento+=sizeof(uint32_t);
                memcpy(&entrada,buffer+desplazamiento,sizeof(uint32_t));
                numeroTabla2 = tabla1ToTabla2(numeroTabla1, entrada);
                usleep(config.RETARDO_MEMORIA * 1000);
                send(socketCPU, &numeroTabla2, sizeof(uint32_t), 0);
                log_info(logger,"Numero de tabla de 2do nivel enviado al CPU");
                break;
            case TABLA2_TO_FRAME:    // size y luego codigo,numeroTabla2(uint32_t),entrada(uint32_t)
                log_info(logger,"El CPU me solicita un numero de frame");
                memcpy(&numeroTabla2,buffer+desplazamiento,sizeof(uint32_t));
                desplazamiento+=sizeof(uint32_t);
                memcpy(&entrada,buffer+desplazamiento,sizeof(uint32_t));
                frame = tabla2ToFrame(numeroTabla2, entrada);
                usleep(config.RETARDO_MEMORIA * 1000);
                send(socketCPU, &frame, sizeof(uint32_t), 0);
                log_info(logger,"Numero de frame enviado al CPU (%d)",frame);
                break;
            case LECTURA:   // size y luego codigo,direccionFisica(uint32_t)
                log_info(logger,"El CPU me solicita una lectura");
                memcpy(&direccionFisica,buffer+desplazamiento,sizeof(uint32_t));
                lectura = realizarLectura(direccionFisica);
                usleep(config.RETARDO_MEMORIA * 1000);
                send(socketCPU, &lectura, sizeof(uint32_t), 0);
                log_info(logger,"Lectura enviada al CPU");
                break;
            case ESCRITURA:
                log_info(logger,"El CPU me solicita una escritura");  // size y luego codigo,direccionFisica(uint32_t),valor(uint32_t)
                memcpy(&direccionFisica,buffer+desplazamiento,sizeof(uint32_t));
                desplazamiento+=sizeof(uint32_t);
                memcpy(&valor,buffer+desplazamiento,sizeof(uint32_t));
                resultado = realizarEscritura(direccionFisica,valor);
                usleep(config.RETARDO_MEMORIA * 1000);
                send(socketCPU, &resultado, sizeof(uint32_t), 0);
                log_info(logger,"Escritura realizada correctamente");
                break;
            default:
                log_info(logger,"Me llegó un mensaje desconocido desde el CPU");
        }
        free(buffer);
    }
}

void esperarMensajesKernel() {
    void* buffer;
    int size, desplazamiento;
    uint32_t numeroTabla;
    // bool resultado=0;
    MENSAJE_KERNEL mensaje;
	while(1) {
        desplazamiento=sizeof(MENSAJE_KERNEL);
        recv(socketKernel, &size, sizeof(int), 0);
        buffer = malloc(size);
        recv(socketKernel, buffer, size, 0);
        memcpy(&mensaje,buffer,sizeof(MENSAJE_KERNEL));
        switch(mensaje) {
            case NUEVO:
                log_info(logger,"Me llego un proceso nuevo desde Kernel");
                PCB* pcb = recibirPCB(buffer, desplazamiento);
                numeroTabla = nuevoProceso(pcb);
                send(socketKernel, &numeroTabla, sizeof(uint32_t), 0);
                log_info(logger,"Numero de tabla enviado a Kernel");
                break;
            case SUSPENDIDO:
                log_info(logger,"Me llego la suspensión de un proceso desde Kernel");
                memcpy(&numeroTabla,buffer+desplazamiento,sizeof(uint32_t));
                suspenderProceso(numeroTabla);
                // send(socketKernel, &resultado, sizeof(bool), 0);
                // log_info(logger,"Resultado de la suspensión enviado a Kernel");
                break;
            case FIN:
                log_info(logger,"Me llego la finalización de un proceso desde Kernel");
                memcpy(&numeroTabla,buffer+desplazamiento,sizeof(uint32_t));
                finProceso(numeroTabla);
                // send(socketKernel, &resultado, sizeof(bool), 0);
                // log_info(logger,"Resultado de la finalización enviado a Kernel");
                break;
            default:
                log_info(logger,"Me llegó un mensaje desconocido desde Kernel");
        }
        free(buffer);
    }
}

int iniciar_servidor(char* ip, char* puerto, int cant_conexiones) {  // cant_conexiones = 0 si acepto todas (SOMAXCONN)
    int socket_servidor;
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    bool conecto = false;

    for (struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
        socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
        setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
        if (socket_servidor == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        conecto = true;
        break;
    }

    if(!conecto) {
        free(servinfo);
        return 0;
    }

    if (cant_conexiones!=0) listen(socket_servidor, cant_conexiones); // Escuchando cant_conexiones conexiones simultaneas
    else listen(socket_servidor, SOMAXCONN); // Escuchando todas las conexiones simultaneas posibles

    freeaddrinfo(servinfo);

    return socket_servidor;
}
