#include <stdio.h>
#include <stdlib.h>

#include "sistemaEntrada.h"
#include "analizadorLexico.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "abb.h"
#include "tablaSimbolos.h"
#include "analizadorSintactico.h"
#include "gestorErrores.h"

#define TAM_BLOQUE 20

void inorden(abb A);



int main() {
    inicializarGestorErrores();


    //Inicializa la tabla de símbolos y la rellenamos con las palabras reservadas
    abb tablaSimbolos;
    crear(&tablaSimbolos);
    insertarPalabrasReservadas(&tablaSimbolos);

    //Imprimimos la información de la tabla de símbolos
    inorden(tablaSimbolos);

    centinela * cent;
    int fd;
    // Iniciamos sistema de entrada
    cent = iniciarSistemaEntrada(TAM_BLOQUE,&fd);
    if(fd != -1){
        // Iniciamos el analizador léxico
        iniciarAnalizadorSintactico(cent,fd,&tablaSimbolos);
        inorden(tablaSimbolos);
        close(fd);
    }

    mostrarErrores();
    free(cent->buffer);
    free(cent);

    destruir(&tablaSimbolos);

    return 0;
}



void _imprimir_lexema(tipoelem E){
    printf("\t || LEXEMA: %-20s || \tVALOR: %3.d\t ||\n",E.lexema,E.componenteLexico);
}

void inorden(abb A){
    tipoelem E;
    if(!es_vacio(A)){
        inorden(izq(A));
        leer(A,&E);
        _imprimir_lexema(E);
        inorden(der(A));
    }
}