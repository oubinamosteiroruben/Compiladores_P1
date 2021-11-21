

#ifndef PRACTICA1_ANALIZADORLEXICO_H
#define PRACTICA1_ANALIZADORLEXICO_H

#include "sistemaEntrada.h"
#include "abb.h"
#include "definiciones.h"
#include "gestorErrores.h"

#define True 1
#define False 0

/**
 *
 * @param cent: centinela donde existe la información de los buffers de entrada
 * @param fd : archivo que se está compilando
 * @param tablaSimbolos :
 * @return devuelve un elemento en el que se encuentra un lexema e información acerca de este
 */
tipoelem siguienteLexema(centinela * cent, int fd, abb * tablaSimbolos);

#endif //PRACTICA1_ANALIZADORLEXICO_H
