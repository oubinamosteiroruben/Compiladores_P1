

#ifndef PRACTICA1_TABLASIMBOLOS_H
#define PRACTICA1_TABLASIMBOLOS_H

#include "abb.h"

/**
 *
 * @param tabla : tabla en la que se van a guardar todas las palabras reservadas
 */
void insertarPalabrasReservadas(abb * tabla);

/**
 *
 * @param E : elemento que se va a insertar en la tabla
 * @param tabla : tabla de símbolos en la que se va a guardar el elemento E
 */
void insertarPalabra(tipoelem E, abb * tabla);

#endif //PRACTICA1_TABLASIMBOLOS_H
