
#ifndef PRACTICA1_SISTEMAENTRADA_H
#define PRACTICA1_SISTEMAENTRADA_H

#define IZQ 0
#define DER 1
#define AMBOS 2

typedef struct{
    char * buffer;
    int inicio;
    int final;
    int lado;
    int ultimaCarga;
    int numLineas;
    int tamMaxBloque;
    int ignore;
} centinela;

/**
 *
 * @param tamBloque : tamaño máximo para el lexema
 * @param fd : archivo que está siendo compilado
 * @return centinela ya inicializada
 */

centinela * iniciarSistemaEntrada(int tamBloque, int * fd);

/**
 *
 * @param cent : centinela donde se encuentra toda la informacioń del doble buffer
 */
void devolverCaracter(centinela * cent);

/**
 *
 * @param cent : centinela donde se encuentra toda la información del doble buffer
 * @param fd : archivo que está siendo compilado
 * @return
 */

char siguienteCaracter(centinela * cent, int fd);

/**
 *
 * @param cent : centinela donde se encuentra toda la información del doble buffer
 */
void empezarLexema(centinela * cent);
#endif //PRACTICA1_SISTEMAENTRADA_H
