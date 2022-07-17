#include "cpu.h"

int main(int argc, char ** argv){
    asignarArchivoConfig(argc, argv);
    iniciar_cpu();
    esperar_finalizacion_hilos();
    finalizar();    
    return 0;
}

void asignarArchivoConfig(int cantidadArgumentos, char** argumentos) {
    pathConfig = string_new();
    if (cantidadArgumentos > 1) {
            string_append(&pathConfig, "./cfg/");
            string_append(&pathConfig, argumentos[1]);
            string_append(&pathConfig, ".config");
    }
    else string_append(&pathConfig, "./cfg/cpu.config");
}

void iniciar_cpu(void){
    lista_tlb = list_create();
    
    pthread_mutex_lock (&interrupt_mutex);
    existe_interrupt = 0;
    pthread_mutex_unlock (&interrupt_mutex);

    iniciar_logger();
    iniciar_config();
    iniciar_TLB();
    iniciar_hilo_comandos();
    iniciar_conexiones();
}

void iniciar_TLB(void){
    int i;    

    for(i = 0; i < config->entradas_tlb; i++){
        TLB* registro_tlb = malloc(sizeof(TLB));
        registro_tlb->numero_pagina = -1;
        registro_tlb->marco = -1;
        registro_tlb->tstamp_creado = time(NULL);
        registro_tlb->tstamp_ultima_vez_usado = time(NULL);

        list_add(lista_tlb, registro_tlb);
    }
}

void limpiar_TLB(void){
    list_clean_and_destroy_elements(lista_tlb, (void*)free);
    iniciar_TLB();
}

int existe_entrada_con_marco(TLB* registro_tlb_nuevo){
    int i;
    TLB * registro_tlb_actual;
    for(i = 0; i < list_size(lista_tlb); i++){
        registro_tlb_actual = list_get(lista_tlb, i);

        if(registro_tlb_actual->marco == registro_tlb_nuevo->marco)
            return i;
    }
    return -1;
}

void actualizar_TLB(TLB* registro_tlb_nuevo){
    registro_tlb_nuevo->tstamp_ultima_vez_usado = time(NULL);

    int indice_reemplazo_tlb = existe_entrada_con_marco(registro_tlb_nuevo);
    if(indice_reemplazo_tlb != -1){
        list_replace_and_destroy_element(lista_tlb, indice_reemplazo_tlb, registro_tlb_nuevo, (void*)free);
    }
    else{
        indice_reemplazo_tlb = verificar_reemplazo_TLB();
        if(indice_reemplazo_tlb == -1){ // No hay lugares vacios
            if(strcmp(config->reemplazo_tlb, "FIFO") == 0){
                reemplazar_TLB_FIFO(registro_tlb_nuevo);
            }
            else if(strcmp(config->reemplazo_tlb, "LRU") == 0){
                reemplazar_TLB_LRU(registro_tlb_nuevo);
            }
        }
        else{ //Hay lugares vacios
            list_replace_and_destroy_element(lista_tlb, indice_reemplazo_tlb, registro_tlb_nuevo, (void*)free);
        }
    }
}

int verificar_reemplazo_TLB(void){
    int i;
    TLB * registro_tlb_actual;

    for(i = 0; i < list_size(lista_tlb); i++){
        registro_tlb_actual = list_get(lista_tlb, i);

        if(registro_tlb_actual->numero_pagina == -1)
            return i;
    }

    return -1;
}

void reemplazar_TLB_FIFO(TLB* registro_tlb_nuevo){
    int i, indice_registro_tlb_mas_viejo = 0;

    TLB* registro_tlb_actual;

    TLB* registro_tlb_mas_viejo = list_get(lista_tlb, 0);

    for(i = 0; i < list_size(lista_tlb); i++){

        registro_tlb_actual = list_get(lista_tlb, i);

        if(difftime(registro_tlb_mas_viejo->tstamp_creado, registro_tlb_actual->tstamp_creado) > 0){
            indice_registro_tlb_mas_viejo = i;
            registro_tlb_mas_viejo = registro_tlb_actual;
        }
    }

    list_replace_and_destroy_element(lista_tlb, indice_registro_tlb_mas_viejo, registro_tlb_nuevo, (void*)free);
}

void reemplazar_TLB_LRU(TLB* registro_tlb_nuevo){
    int i, indice_registro_tlb_menos_recientemente_usado = 0;

    TLB* registro_tlb_actual;

    TLB* registro_tlb_menos_recientemente_usado = list_get(lista_tlb, 0);

    for(i = 0; i < list_size(lista_tlb); i++){

        registro_tlb_actual = list_get(lista_tlb, i);

        if(difftime(registro_tlb_menos_recientemente_usado->tstamp_ultima_vez_usado, registro_tlb_actual->tstamp_ultima_vez_usado) > 0){
            indice_registro_tlb_menos_recientemente_usado = i;
            registro_tlb_menos_recientemente_usado = registro_tlb_actual;
        }
    }
    
    list_replace_and_destroy_element(lista_tlb, indice_registro_tlb_menos_recientemente_usado, registro_tlb_nuevo, (void*)free);
}

void intercambio_con_kernel(){
    int check_interrupt;
    int flag_fin;
    while(1){

        PCB* pcb = recibirPCB();
        if(id_ultimo_PCB_usado != pcb->id){
            limpiar_TLB();
            id_ultimo_PCB_usado = pcb->id; //Ver si necesito un mutex
        }
        flag_fin = 0;
        check_exit = 0;
        check_io = 0;
        pthread_mutex_lock (&interrupt_mutex);
        existe_interrupt = 0;
        pthread_mutex_unlock (&interrupt_mutex);

        while(flag_fin == 0){
            exec_ciclo_instruccion(pcb);

            check_interrupt = chequear_interrupciones();
            if (check_interrupt==1) log_info(logger,"Interrupción recibida!");
            if (check_exit == 1 || check_io == 1) id_ultimo_PCB_usado=-1;
            if(check_exit == 1 || check_io == 1 || check_interrupt == 1){
                flag_fin = 1;
                enviarPCB(pcb);                
            }
            else{
                pcb->contador = pcb->contador + 1;
                no_op_restantes[pcb->id]=-1;
            }
        }

        if(check_interrupt == 1){
            pthread_mutex_lock (&interrupt_mutex);
            existe_interrupt = 0;
            pthread_mutex_unlock (&interrupt_mutex);
        }
        destruirPCB(pcb);
    }
}

void exec_ciclo_instruccion(PCB *pcb){
    uint32_t valorLeidoParaCopy=0;
    Instruccion* prox_instruccion = fetch(pcb);
    if(decode(prox_instruccion) == 1) valorLeidoParaCopy = fetch_operands(pcb, prox_instruccion);
    execute(pcb,prox_instruccion,valorLeidoParaCopy);
}

Instruccion* fetch(PCB *pcb){
    Instruccion* prox_instruccion = list_get(pcb->instrucciones, pcb->contador);
    return prox_instruccion;
}

int decode(Instruccion* prox_instruccion){
    if (prox_instruccion->identificador == COPY) return 1;
    else return 0;
}

uint32_t fetch_operands(PCB* pcb, Instruccion* prox_instruccion){
    uint32_t dir_fisica, result;
    dir_fisica = obtener_direccion_fisica(pcb->tabla, prox_instruccion->parametro2); 
    result = enviar_operacion_a_memoria(LECTURA,dir_fisica,prox_instruccion);
    return result;
}

void execute(PCB *pcb, Instruccion* prox_instruccion, uint32_t valorLeidoParaCopy){
    int i, checkInt;
    int flag=1;
    uint32_t dir_fisica, result;
    switch(prox_instruccion->identificador){
        case NO_OP:
                    if (no_op_restantes[pcb->id]==-1) no_op_restantes[pcb->id]=prox_instruccion->parametro1;
                    for (i=0; i<no_op_restantes[pcb->id] && flag>0; i++) {
                        log_info(logger, "Ejecutando instrucción NO_OP para proceso %d", pcb->id);   
                        usleep(config->retardo_noop * 1000);
                        checkInt = chequear_interrupciones();
                        if (checkInt==1) {
                            flag=0;
                            no_op_restantes[pcb->id]-=(i+1);
                        }
                    }
                    break;
        case IO:
                    log_info(logger, "Ejecutando instrucción IO para proceso %d", pcb->id);
                    check_io = 1;
                    break;
        case READ:
                    log_info(logger, "Ejecutando instrucción READ para proceso %d", pcb->id);
                    //Obtengo dir fisica, mando tabla y dir logica
                    dir_fisica = obtener_direccion_fisica(pcb->tabla, prox_instruccion->parametro1); 
                    //Envío operacion, el 2 es el cod de la operacion lectura
                    result = enviar_operacion_a_memoria(LECTURA,dir_fisica,prox_instruccion);
                    log_info(logger, "Valor leido de memoria: %d", result);
                    
                    break;
        case WRITE:                    
                    log_info(logger, "Ejecutando instrucción WRITE para proceso %d", pcb->id);
                    //Obtengo dir fisica, mando tabla y dir logica
                    dir_fisica = obtener_direccion_fisica(pcb->tabla, prox_instruccion->parametro1); 
                    //Envío operacion, el 2 es el cod de la operacion lectura
                    result = enviar_operacion_a_memoria(ESCRITURA,dir_fisica,prox_instruccion);
                    if(result != 0){
                        log_error(logger, "Error al intentar escribir en la posicion %d\n", dir_fisica);
                    }
                    else{
                        log_trace(logger, "Escritura exitosa del valor %d en la posicion de memoria %d\n", prox_instruccion->parametro2, dir_fisica);                        
                    }
                    
                    break;
        case COPY:
                    log_info(logger, "Ejecutando instrucción COPY para proceso %d", pcb->id);
                    dir_fisica = obtener_direccion_fisica(pcb->tabla, prox_instruccion->parametro1); 
	                prox_instruccion->parametro2 = valorLeidoParaCopy;
                    result = enviar_operacion_a_memoria(ESCRITURA,dir_fisica,prox_instruccion);
                    break;
        case EXIT:
                    log_info(logger, "Ejecutando instrucción EXIT para proceso %d", pcb->id);
                    check_exit = 1;
                    break;
        default:
                    break;
    }
}

int chequear_interrupciones(){
    int check_interrupt_aux;

    pthread_mutex_lock (&interrupt_mutex);
    check_interrupt_aux = existe_interrupt;
    pthread_mutex_unlock (&interrupt_mutex);

    return check_interrupt_aux;
}

TLB* buscar_en_TLB(uint32_t numero_pagina){
    int i;

    TLB* registro_tlb;


    for(i = 0; i < list_size(lista_tlb); i++){

        registro_tlb = list_get(lista_tlb, i);

        if(registro_tlb->numero_pagina == numero_pagina)
            return registro_tlb;
    }

    return NULL;
}

void finalizar(void) {
	log_info(logger,"Finalizando el modulo CPU");
	log_destroy(logger);
	config_destroy(auxConfig);
	close(socket_servidor_dispatch);
	close(socket_servidor_interrupt); 
    close(socket_cpu_memoria);
    close(socket_pcb); 
    close(socket_interrupcion); 
    close(socket_cliente_dispatch); 
    close(socket_cliente_interrupt);  
//	... Hacer los free/close/destroy
}

void intercambio_con_memoria(void){

    uint32_t handshake = 0;
    uint32_t result;

    send(socket_cpu_memoria, &handshake, sizeof(uint32_t), 0);
    recv(socket_cpu_memoria, &result, sizeof(uint32_t), 0);

    if(result != 0){
        log_error(logger, "Error al intentar handshake con Memoria");
    }
    else{
        recibir_config_memoria();
    }

}

void recibir_config_memoria() {
    config_memoria = malloc(sizeof(mem_config));
    int desplazamiento = 0, size;
    uint32_t ent_tabla_de_pag, tam_pag;

    recv(socket_cpu_memoria, &size, sizeof(int), 0);
	void* buffer = malloc(size);
    recv(socket_cpu_memoria, buffer, size, 0);

    memcpy(&ent_tabla_de_pag,buffer+desplazamiento,sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);
    memcpy(&tam_pag,buffer+desplazamiento,sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);

    config_memoria->cant_entradas_por_tabla_de_paginas = ent_tabla_de_pag;
    config_memoria->tamanio_pagina = tam_pag;

    log_info(logger,"Configuracion recibida desde memoria");
    free(buffer);
}

void esperar_finalizacion_hilos() {
    pthread_join(hiloServerDispatch,NULL);
    pthread_join(hiloServerInterrupt,NULL);
    pthread_join(hiloClienteMemoria,NULL);
}

uint32_t obtener_direccion_fisica(uint32_t tabla_1, uint32_t dir_logica ){     
    int result_tabla2, result_frame;
    uint32_t entrada1 = calcular_entrada1(dir_logica);
    uint32_t entrada2 = calcular_entrada2(dir_logica);
    uint32_t desplazamiento = calcular_desplazamiento(dir_logica);
    uint32_t direccion_fisica;
    uint32_t numero_pagina = floor(dir_logica / config_memoria->tamanio_pagina);
    int mensaje_tabla = TABLA1_TO_TABLA2;
    int mensaje_frame = TABLA2_TO_FRAME;
    TLB* registro_buscado = buscar_en_TLB(numero_pagina);
    if(registro_buscado != NULL){ //Existe la pagina en TLB
        registro_buscado->tstamp_ultima_vez_usado =time(NULL);
        // actualizar_tstamp_pagina(registro_buscado);
    }
    else{ //No esta en la TLB
        registro_buscado = malloc(sizeof(TLB));
        result_tabla2 = solicitar_direcciones_memoria(tabla_1, entrada1, mensaje_tabla );
        result_frame = solicitar_direcciones_memoria(result_tabla2, entrada2,mensaje_frame);
        registro_buscado->marco = result_frame;
        registro_buscado->numero_pagina = numero_pagina;
        registro_buscado->tstamp_creado = time(NULL);
        registro_buscado->tstamp_ultima_vez_usado =time(NULL);
        actualizar_TLB(registro_buscado);
    }
    //Caluclo la dirección fisica a enviar a memoria
    direccion_fisica = traducir_direccion_recibida(config_memoria->tamanio_pagina, registro_buscado->marco, desplazamiento);
    return direccion_fisica;
}

uint32_t enviar_operacion_a_memoria(int operacion_principal, uint32_t dir_fisica, Instruccion* prox_instruccion){                         
    uint32_t result;
    uint32_t valor_a_escribir;
    int sizeTotal;
    int desplazamiento = 0;
    if(operacion_principal == ESCRITURA){
        valor_a_escribir = prox_instruccion->parametro2;
        sizeTotal = sizeof(uint32_t)*2 +sizeof(int);
    }else{
        sizeTotal = sizeof(uint32_t) +sizeof(int);
    }
    void* stream = malloc(sizeTotal);
    desplazamiento = 0;
    memcpy(stream + desplazamiento, &operacion_principal, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(stream + desplazamiento, &dir_fisica, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    if(operacion_principal == ESCRITURA){
        memcpy(stream + desplazamiento, &valor_a_escribir, sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t);
    }
    
    send(socket_cpu_memoria, &sizeTotal, sizeof(int), 0); //mando el size
    //Indico operacion a realizar va a ser 2 (lectura) o 3 (Escritura)
    send(socket_cpu_memoria, stream, sizeTotal, 0);
    //Recibo el resultado
    recv(socket_cpu_memoria, &result, sizeof(uint32_t), 0);
    free(stream);
    return result;
}

uint32_t traducir_direccion_recibida(uint32_t tamanio_pagina,  uint32_t numero_frame, uint32_t desplazamiento){
    uint32_t dir_fisica = tamanio_pagina * numero_frame + desplazamiento;
    return dir_fisica;
}

//revisar estas funciones que robe a bruno
uint32_t calcular_entrada1(uint32_t dir_logica){
        uint32_t numero_pagina = floor(dir_logica / config_memoria->tamanio_pagina);
        uint32_t entrada1 = floor(numero_pagina / config_memoria->cant_entradas_por_tabla_de_paginas);
    return entrada1;
}

uint32_t calcular_entrada2(uint32_t dir_logica){
        uint32_t numero_pagina = (uint32_t) floor((double)(dir_logica / config_memoria->tamanio_pagina));
        uint32_t entrada2 = (uint32_t) numero_pagina % config_memoria->cant_entradas_por_tabla_de_paginas;
    return entrada2;
}

uint32_t calcular_desplazamiento(uint32_t dir_logica){
        uint32_t numero_pagina = (uint32_t) floor((double)(dir_logica / config_memoria->tamanio_pagina));
        uint32_t desplazamiento = dir_logica - numero_pagina * config_memoria->tamanio_pagina;
    return desplazamiento;
}
