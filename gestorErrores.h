
#ifndef PRACTICA1_GESTORERRORES_H
#define PRACTICA1_GESTORERRORES_H


/**
 *
 * @param numLinea
 * @param codigo
 * @param lexema
 * @param caracter
 */

void nuevoError(int numLinea, int codigo, char * lexema, char caracter);

/**
 * Muestra los errores que se fueron cargando durando la compilación
 */
void mostrarErrores();

/**
 * Inicia el gestor de errores
 */
void inicializarGestorErrores();

#endif //PRACTICA1_GESTORERRORES_H
