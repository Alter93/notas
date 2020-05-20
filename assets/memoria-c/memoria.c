//
// memoria.c
//
// Alejandro Alvarez
// Copyright (c) 2020. All rights reserved.
//

#include <stdio.h>
#include "memoria.h"

static byte *memoria = NULL;
Cabecera *inicio_memoria;
u_int64_t tamano_total;

void iniciar(u_int64_t tamano){
    // La funcion malloc, al asignarla a la variable memoria, de tipo *byte,
    // solicita M veces el tamaño de byte definido en las primeras linea
    memoria = malloc(tamano);
    tamano_total = tamano;
    inicio_memoria = (Cabecera*)memoria;

    // Identificador de memoria vacia 0x0000000
    inicio_memoria->id = 0;

    inicio_memoria->tamano = tamano;
    inicio_memoria->siguiente = NULL;
}

void *reservar_memoria(u_int64_t n){
    Cabecera * indice_memoria = inicio_memoria;

    while (indice_memoria != NULL){
        // Recorrer lista hasta encontrar espacio vacio
        if (indice_memoria->id == 0){
            // Necesitamos que la memoria sea capaz de guardar
            // al menos una cabecera y los datos que le pedimos
            if (indice_memoria->tamano >= (n + TAMANO_CABECERA)){
                break;
            }
        }
        indice_memoria = indice_memoria->siguiente;
    }

    if (indice_memoria != NULL){
        // Fragmentar memoria
        // Al dividir la memoria a la mitad, necesita caber
        // una cabecera y los datos solicidatos en cada mitad
        // esta operacion se realiza hasta encontrar la potencia de 2
        // que se ajuste mejor al espacio solicitado
        while (indice_memoria->tamano / 2 >= (n + TAMANO_CABECERA)){
            indice_memoria->tamano = indice_memoria->tamano / 2;
            // Encontrar inicio del segundo bloque usando aritmetica de
            // apuntadores
            byte * ptr = (byte*)indice_memoria + indice_memoria->tamano;
            Cabecera *bloque_nuevo = (Cabecera*)ptr;
            bloque_nuevo->siguiente = indice_memoria->siguiente;
            bloque_nuevo->id = 0;
            bloque_nuevo->tamano = indice_memoria->tamano;
            indice_memoria->siguiente = bloque_nuevo;
        }
        // id programa ficticio
        indice_memoria->id = 0x12345678;
    }

    return indice_memoria;
}

void liberar_memoria(void *objeto){
    Cabecera*bloque_memoria = (Cabecera*)objeto;
    bloque_memoria->id = 0;
    unir_espacio(memoria);
}

void unir_espacio(byte *inicio){
    Cabecera*info = (Cabecera*)inicio;
    if(inicio + info->tamano >= memoria + tamano_total){
        // No hay siguiente bloque
        return;
    }
    else if(info->siguiente->tamano == info->tamano && info->siguiente->id == 0){
        // Unir info y info->siguiente
        info->tamano = info->tamano * 2;
        info->siguiente = info->siguiente->siguiente;

        unir_espacio(inicio);

    }
    else {
        byte * siguiente = inicio + info->tamano;
        unir_espacio(siguiente);
    }
}


void finalizar(void){
    // libera la memoria reservada
    free(memoria);
}

void imprimir_lista(){
    Cabecera* inicio = (Cabecera*)memoria;
    printf("       NULL\n");
    printf("         |\n");
    while(inicio != NULL){
        printf("------------------\n");
        byte * dir_inicio = (byte*)inicio;
        printf("ID: 0x%llx\n", inicio->id);
        printf("Tamaño: %llu\n", inicio->tamano);
        printf("Direccion inicio: %p\n", dir_inicio);
        printf("Direccion fin: %p\n", &dir_inicio[inicio->tamano-1]);

        // con la bandera -fsanitize=address las dos lineas inferiores
        // comprueban que estamos accediendo a regiones de la
        // memoria que reservamos
        dir_inicio[0] = 0;
        dir_inicio[inicio->tamano-1] = 0;

        printf("------------------\n");
        printf("         |\n");
        inicio = inicio->siguiente;
    }
    printf("       NULL\n");
}

int main(){
    u_int64_t tamano = 1024;
    iniciar(tamano);
    void*ptr1 = reservar_memoria(8);
    void*ptr2 = reservar_memoria(100);
    void*ptr3 = reservar_memoria(200);
    void*ptr4 = reservar_memoria(32);

    imprimir_lista();
    liberar_memoria(ptr1);
    liberar_memoria(ptr2);
    liberar_memoria(ptr3);
    liberar_memoria(ptr4);
    printf("\n\nLiberar todo el espacio...\n");
    imprimir_lista();

    return 0;
}
