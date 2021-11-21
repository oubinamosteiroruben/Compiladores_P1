

#include "analizadorSintactico.h"
#include "analizadorLexico.h"
#include "tablaSimbolos.h"
#include "gestorErrores.h"
#include <stdio.h>
#include <stdlib.h>


void iniciarAnalizadorSintactico(centinela * cent, int fd, abb * tablaSimbolos){
    tipoelem E;
    printf("\nLista Componentes Léxicos:\n\n");
    // El analizador sintáctico le pedirá todos los componentes léxicos al analizador léxico
    do{
        // Se genera el siguiente componente léxico
        E = siguienteLexema(cent, fd,tablaSimbolos);
        // Si el componente léxico es el final del documento, se indicará y finalizará la búsqueda
        if(E.eof){
            printf("\t   ELEM: %-20s  \t--> CL: %3d\n","EOF",E.componenteLexico);
            printf("\t------------------------------------------------\n");
        }else if(E.componenteLexico!=-1){
            // Se imprimen los componentes léxicos recibidos
            if(E.componenteLexico == CL_SALTO){
                printf("\t   ELEM: %-20s  \t--> CL: %3d\n","Nueva Linea",E.componenteLexico);
            }else if(E.componenteLexico == CL_TAB) {
                printf("\t   ELEM: %-20s  \t--> CL: %3d\n","Tabulador",E.componenteLexico);
            }else{
                    printf("\t   ELEM: %-20s  \t--> CL: %3d\n",E.lexema,E.componenteLexico);
            }
            printf("\t------------------------------------------------\n");
        }
        //Se libera el lexema previamente reservado en memoria dinámicamente
        free(E.lexema);
    }while(!E.eof);
}