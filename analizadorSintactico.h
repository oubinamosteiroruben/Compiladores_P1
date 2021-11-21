
#ifndef PRACTICA1_ANALIZADORSINTACTICO_H
#define PRACTICA1_ANALIZADORSINTACTICO_H


#include "sistemaEntrada.h"
#include "abb.h"
#include "gestorErrores.h"

/**
 *
 * @param cent : centinela donde se guarda la información del sistema de entrada
 * @param fd : archivo que se está compilando
 * @param tablaSimbolos
 */
void iniciarAnalizadorSintactico(centinela * cent, int fd, abb * tablaSimbolos);

#endif //PRACTICA1_ANALIZADORSINTACTICO_H
