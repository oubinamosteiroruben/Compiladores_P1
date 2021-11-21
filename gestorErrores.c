
#include "gestorErrores.h"
#include <stdio.h>
#include <stdlib.h>

// Lista de errores
char * cadenaErrores[40] = {"Final inesperado.", "Carácter no válido.","El tamaño del lexema excede el límite."};

// Funció nque se encargará de enviar información para que el error pueda decir donde está teniendo lugar y el motivo
void nuevoError(int numLinea, int codigo, char * lexema, char caracter){
    FILE * fd;
    fd = fopen("Errores.txt","a");
    if(fd){
        if(codigo == 1){
            fprintf(fd,"ID Error: %d.\t LINEA: %d.\t Error: %s --> %s<%c>\n",codigo,numLinea,cadenaErrores[codigo],lexema,caracter);
        }else{
            fprintf(fd,"ID Error: %d.\t LINEA: %d.\t Error: %s\n",codigo,numLinea,cadenaErrores[codigo]);
        }
        fclose(fd);
    }
}

// Función que elimina, en caso de que exista, un archivo en el que se registrán todos los errores
void inicializarGestorErrores(){
    remove("Errores.txt");
}

// Muestra el registro de todos los errores que han tenido lugar
void mostrarErrores(){
    FILE * fd;
    char cadena[80];
    fd = fopen("Errores.txt","r");
    printf("\nRegistro de Errores\n\n");
    if(fd){
        while (fgets(cadena, 80, fd)){
            printf("%s",cadena);
        }
        fclose(fd);
    }
}
