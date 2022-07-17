#include "conexiones.h"

void abrirServidorParaProcesos() {
	socketServer = iniciar_servidor(NULL, config.PUERTO_ESCUCHA,0); // NULL = "127.0.0.1"
	if (pthread_create(&hiloServer, NULL, (void*) &esperarProcesos, NULL) != 0) {
		log_error(logger, "Error al crear el hilo del servidor");
	}
	else log_info(logger,"Servidor iniciado y esperando que lleguen procesos");
}

void conectarConMemoria() {
    int handshake = 1;
    socketMemoria = conectarAServidor (config.IP_MEMORIA, config.PUERTO_MEMORIA);
    send(socketMemoria,&handshake, sizeof(int), 0);
    recv(socketMemoria,&handshake, sizeof(int), 0);
    log_info(logger,"Conexion exitosa con Memoria");
}

void conectarConCPU() {
    socketDispatch = conectarAServidor (config.IP_CPU, config.PUERTO_CPU_DISPATCH);
    if (pthread_create(&hiloDispatch, NULL, (void*) &intercambioConDispatch, NULL) != 0) {
		log_error(logger, "Error al crear el hilo de CPU-Dispatch");
	}
	else log_info(logger,"Conexion exitosa con CPU-Dispatch");

    socketInterrupt = conectarAServidor (config.IP_CPU, config.PUERTO_CPU_INTERRUPT);        // COMENTAR SI NO TENGO INTERRUPT
    log_info(logger,"Conexion exitosa con CPU-INTERRUPT");                                   // COMENTAR SI NO TENGO INTERRUPT
}

void esperarProcesos (){
	while(1) {
        socketProceso[idProximoProceso] = accept(socketServer, NULL, NULL);
        log_info(logger,"Nuevo proceso recibido");
        nuevoProceso();
    }
}

void nuevoProceso() {
	PCB* pcb = recibirProcesoYGenerarPCB();
    list_add(listaNew,pcb);
    list_add(listaDeTodosMisProcesos,pcb);
    log_info(logger,"Pongo en new al proceso %d",idProximoProceso-1);
    sem_post(&semCantidadProcesosEnNew);
}

int conectarAServidor (char* ip, char* puerto) {
	struct addrinfo hints;
	struct addrinfo *server_info;
	int clientSocket;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(ip, puerto, &hints, &server_info)){   // error
        log_error(logger, "ERROR EN GETADDRINFO");
        exit(-1);
    }

    clientSocket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    setsockopt(clientSocket, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

    if(connect(clientSocket, server_info->ai_addr, server_info->ai_addrlen) == -1){
        if (puerto == config.PUERTO_MEMORIA) log_error(logger, "FALLO LA CONEXION CON MEMORIA...");
        else if (puerto == config.PUERTO_CPU_DISPATCH) log_error(logger, "FALLO LA CONEXION CON DISPATCH...");
        else if (puerto == config.PUERTO_CPU_INTERRUPT) log_error(logger, "FALLO LA CONEXION CON INTERRUPT...");
        else log_error(logger, "FALLO LA CONEXION CON ALGO DESCONOCIDO...");
        exit(-1);
    }

    freeaddrinfo(server_info);
    return clientSocket;
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

void destruirListasConSusElementos() {
    list_destroy_and_destroy_elements(listaNew,(void*)destruirPCB);
    list_destroy_and_destroy_elements(listaReady,(void*)destruirPCB);
    list_destroy_and_destroy_elements(listaExec,(void*)destruirPCB);
    list_destroy_and_destroy_elements(listaExit,(void*)destruirPCB);
    list_destroy_and_destroy_elements(listaBlocked,(void*)destruirPCB);
    list_destroy_and_destroy_elements(listaSuspendedReady,(void*)destruirPCB);
    list_destroy_and_destroy_elements(listaSuspendedBlocked,(void*)destruirPCB);
    list_destroy_and_destroy_elements(listaPendientesDeDesbloqueo,(void*)destruirPCB);
}
