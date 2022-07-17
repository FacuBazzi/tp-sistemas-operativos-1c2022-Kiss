#include "cpu.h"

void iniciar_conexiones(void){

    iniciar_cliente_memoria();
    iniciar_servidores();
}

void iniciar_servidores(void){
    socket_servidor_dispatch = crear_servidor("DISPATCH", NULL, config->puerto_escucha_dispatch);
    socket_servidor_interrupt = crear_servidor("INTERRUPT", NULL, config->puerto_escucha_interrupt);    

    //Creo el hilo para esperar los PCBs desde kernel
	if (pthread_create(&hiloServerDispatch, NULL, (void*) &esperar_pcbs, &socket_servidor_dispatch) != 0) {
		log_error(logger, "Error al crear el hilo del servidor de dispatch en CPU");
	}
	else log_info(logger,"Servidor dispatch CPU iniciado, esperando cliente");

    //Creo el hilo para esperar las interrupciones desde Kernel
    if (pthread_create(&hiloServerInterrupt, NULL, (void*) &esperar_interrupciones, &socket_servidor_interrupt) != 0) {
		log_error(logger, "Error al crear el hilo del servidor de interrupt en CPU");
	}
	else log_info(logger,"Servidor interrupt CPU iniciado, esperando cliente");   
}

void iniciar_cliente_memoria(void){
    socket_cpu_memoria = crear_cliente(config->ip_memoria, config->puerto_memoria);

    if (pthread_create(&hiloClienteMemoria, NULL, (void*) &intercambio_con_memoria, NULL) != 0) {
        log_error(logger, "Error al crear el hilo de CPU-Memoria");
	}
	else log_info(logger,"Conexion exitosa con CPU-Memoria");
    
    //TODO - Mandar mensaje de memoria para traer tamaño de la TLB
    
}

int crear_servidor(char* nombre, const char* ip, char* puerto) {
    int socket_servidor;
    struct addrinfo hints, *servinfo;

    // Inicializando hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Recibe los addrinfo
    getaddrinfo(ip, puerto, &hints, &servinfo);

    bool conecto = false;

    // Itera por cada addrinfo devuelto
    for (struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
        socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
        setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
        if (socket_servidor == -1) // fallo de crear socket
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            // Si entra aca fallo el bind
            close(socket_servidor);
            continue;
        }
        // Ni bien conecta uno nos vamos del for
        conecto = true;
        break;
    }

    if(!conecto) {
        free(servinfo);
        return 0;
    }

    listen(socket_servidor, SOMAXCONN); // Escuchando (hasta SOMAXCONN conexiones simultaneas)

    // Aviso al logger
    // log_info(logger, "Escuchando en %s:%s (%s)\n", ip, puerto, nombre);

    freeaddrinfo(servinfo); //free

    return socket_servidor;
}

void esperar_pcbs(void* socket_servidor){
    socket_pcb = accept((int)socket_servidor_dispatch, NULL, NULL);
    log_info(logger, "Dispatch conectado, esperando que lleguen PCBs");
    intercambio_con_kernel();
}

void esperar_interrupciones(void* socket_servidor){
    socket_interrupcion = accept((int)socket_servidor_interrupt, NULL, NULL);
    log_info(logger, "Interrupt conectado, esperando recibir interrupciones");
    int nuevo;
    while(1) {
        recv(socket_interrupcion, &nuevo, sizeof(int), 0);
        avisar_interrupcion();
    }
}

void avisar_interrupcion(void){
    pthread_mutex_lock (&interrupt_mutex);
    existe_interrupt = 1;
    pthread_mutex_unlock (&interrupt_mutex);
}

int crear_cliente(char *ip, char* puerto){
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family,
							server_info->ai_socktype,
							server_info->ai_protocol);
    setsockopt(socket_cliente, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    setsockopt(socket_cliente, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
	// Ahora que tenemos el socket, vamos a conectarlo
	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
		printf("Error\n");
		exit(3);
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

uint32_t solicitar_direcciones_memoria(uint32_t tabla_obtenida, uint32_t entrada_obtenida, int cod_mensaje ){
    //int size = sizeof(uint32_t)*3;
    int sizeTotal = sizeof(uint32_t)*2 +sizeof(MENSAJE_PARA_MEMORIA);
    int desp = 0;
    uint32_t numero_estructura_recibida;

    //Consulto a memoria por el numero de tabla 2, envío tabla 1 y la entrada a esa tabla
    send(socket_cpu_memoria, &sizeTotal, sizeof(int), 0); //mando el size

    //Serializo mensaje para memoria
    void* stream = malloc(sizeTotal);
    memcpy(stream + desp, &cod_mensaje, sizeof(int));
    desp += sizeof(int);
    memcpy(stream + desp, &tabla_obtenida, sizeof(uint32_t));
    desp += sizeof(uint32_t);
    memcpy(stream + desp, &entrada_obtenida, sizeof(uint32_t));
    desp += sizeof(uint32_t);

    //Envio mensaje a memoria para recibir la proxima estructura
    send(socket_cpu_memoria, stream, sizeTotal, 0);           
    //Recibo estructura
    recv(socket_cpu_memoria, &numero_estructura_recibida, sizeof(uint32_t), 0);
    free(stream);
    return numero_estructura_recibida;
}

void matarHilos() {        // finaliza a los hilos en caso de querer terminar la ejecucion para que siga el camino feliz
    pthread_cancel(hiloClienteMemoria);
    pthread_cancel(hiloServerInterrupt);
    pthread_cancel(hiloServerDispatch);
}

void recibirComandos() {    // para que el modulo lea comandos
    int fin=0;
    char* lectura;
    while(!fin) {
        lectura=readline("");
        if(!strcmp(lectura,"fin")) fin=1;     // le pueden agregar mas comandos para testear o manejar el modulo
        free(lectura);
    }
    matarHilos();
}

void iniciar_hilo_comandos() {
    pthread_create(&hiloParaComandos, NULL, (void*) &recibirComandos, NULL);
    pthread_detach(hiloParaComandos);
}
