//
// memoria.h
//
// Alejandro Alvarez
// Copyright (c) 2020. All rights reserved.
//

#ifndef memoria_memoria_h
#define memoria_memoria_h

#include <stdlib.h>

typedef uint8_t byte;

typedef struct _header Cabecera;
typedef struct _header{
    u_int64_t id;
    u_int64_t tamano;
    Cabecera *siguiente;
}header;

u_int64_t TAMANO_CABECERA = 2 * sizeof(u_int64_t) + sizeof(Cabecera*);

void iniciar(u_int64_t tamano);

void *reservar_memoria(u_int64_t n);

void liberar_memoria(void *objeto);

void finalizar(void);

void unir_espacio(byte *inicio);

void imprimir_lista();

#endif
