#include "instruccion.h"

extern t_log* logger;

char* leer_instrucciones(char* ruta){
    FILE* file = fopen(ruta, "r");
    char* instrucciones = NULL;
    int string_size, read_size;

    if (file == NULL) {
        log_error(logger, "El archivo de instrucciones no existe");
		return NULL;
	}
       fseek(file, 0, SEEK_END);
       string_size = ftell(file);
       rewind(file);
       instrucciones = (char*) malloc(sizeof(char) * (string_size + 1) );
       read_size = fread(instrucciones, sizeof(char), string_size, file);
       instrucciones[string_size] = '\0';

       if (string_size != read_size)
       {
           free(instrucciones);
           instrucciones = NULL;
       }
       fclose(file);
    return instrucciones;
}

t_list* cargar_instrucciones(char* instrucciones){
    t_list* lista_instrucciones = list_create();
    int posicion_cursor = 0;
    char** instrucciones_separadas = string_split(instrucciones, "\n");
    char* ins;
    int flag = 1;
    int i;
    uint32_t valor_parametro = 0;
    uint32_t valor_parametro2 = 0;    
    t_instruccion* instruccion = NULL;   
    for (i=0; flag; i++) {
        if (instrucciones_separadas[i] == NULL) {flag=0; free(instrucciones_separadas[i]);}
        else {
            ins = instrucciones_separadas[i];
            switch(ins[0]) {
                case 'N':
                    valor_parametro = leer_numero(ins, &posicion_cursor, 1);
                    instruccion = inicializar_instruccion(NO_OP, valor_parametro, 0);
                    list_add(lista_instrucciones, (t_instruccion*) instruccion);
                    break;
                case 'I':
                    valor_parametro = leer_numero(ins, &posicion_cursor, 1);
                    instruccion = inicializar_instruccion(IO, valor_parametro, 0);
                    list_add(lista_instrucciones, instruccion);
                    break;
                case 'R': 
                    valor_parametro = leer_numero(ins, &posicion_cursor, 1);
                    instruccion = inicializar_instruccion(READ, valor_parametro, 0);
                    list_add(lista_instrucciones, instruccion);
                    break;
                case 'W':
                    valor_parametro = leer_numero(ins, &posicion_cursor, 1);
                    valor_parametro2 = leer_numero(ins, &posicion_cursor, 2);                
                    instruccion = inicializar_instruccion(WRITE, valor_parametro, valor_parametro2);
                    list_add(lista_instrucciones, instruccion);                
                    break;
                case 'C':
                    valor_parametro = leer_numero(ins, &posicion_cursor, 1);                                
                    valor_parametro2 = leer_numero(ins, &posicion_cursor, 2); 
                    instruccion = inicializar_instruccion(COPY, valor_parametro, valor_parametro2);
                    list_add(lista_instrucciones, instruccion);
                    break;
                case 'E':
                    instruccion = inicializar_instruccion(EXIT, 0, 0); 
                    list_add(lista_instrucciones, instruccion);
                    break;
            }
            free(ins);
        }
    }
    free(instrucciones_separadas);
    return lista_instrucciones;
}

uint32_t leer_numero(char* linea_instruccion, int *posicion_cursor, int param){    
    char** palabras = string_split(linea_instruccion, " ");
    uint32_t numero;
    if (param == 1) {
        numero = atoi(palabras[1]);
    }
    else {
        numero = atoi(palabras[2]);
        free(palabras[3]);
    }
    free(palabras[0]);
    free(palabras[1]);
    free(palabras[2]);
    free(palabras);
    return numero; 
}

t_instruccion* inicializar_instruccion(IDENTIFICADOR_INSTRUCCION identificador, uint32_t primer_parametro, uint32_t segundo_parametro){
    t_instruccion* instruccion = malloc(sizeof(t_instruccion)); 
    instruccion->identificador = identificador;
    instruccion->primer_parametro = primer_parametro;
    instruccion->segundo_parametro = segundo_parametro;
    return instruccion;
}

void liberar_instruccion(t_instruccion* instruccion){
    free(instruccion);
}
