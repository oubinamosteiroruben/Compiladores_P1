#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tablaSimbolos.h"
#include "string.h"
#include "abb.h"
#include <ctype.h>

#include "palabrasReservadas.h"

#define TAM_MAX 20

// Función que carga todas las palabras reservadas, calcula su vu valor (Componente Léxico), y las inserta en la tabla
void insertarPalabrasReservadas(abb * tabla){
    FILE * fd;
    char cadena[TAM_MAX];
    int i = 0;
    tipoelem E;
    // Abrimos el fichero donde se encuentra la lista de palabras reservadas
    fd = fopen("palabrasReservadas.txt","r");
    if(fd){
        do{
            // Se lee cada una de las lineas del archivo, las cuales se asociarán con un ID
            fscanf(fd,"%s",cadena);
            E.lexema = (char*)malloc(sizeof(char)*(strlen(cadena)+1));
            strcpy(E.lexema,cadena);
            E.lexema[strlen(cadena)] = '\0';
            E.componenteLexico = VALOR_INICIAL+i;
            // En caso de que no exista en la tabla de símbolos, se inserta
            if(!es_miembro(*tabla,E)){
                insertar(tabla,E);
            }else{
                free(E.lexema);
            }
            i++;
        }while(fgetc(fd)!=EOF);
        fclose(fd);
    }
}

// Se  inserta un elemento E, en la tabla de simbolos
void insertarPalabra(tipoelem E, abb * tabla){
    insertar(tabla,E);
}
