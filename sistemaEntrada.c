
#include "sistemaEntrada.h"
#include "abb.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "analizadorLexico.h"

char * getBloque(centinela * cent, int fd);

void actualizarBloqueLado(centinela * cent, int fd);

// Función para iniciar el sistema de entrada, el cual inicializará el centinela y abrirá el archivo

centinela * iniciarSistemaEntrada(int tamBloque, int * fd){
    centinela * cent;
    *fd = open("wilcoxon.py",O_RDONLY);
    if((*fd) == -1){
        printf("Error al abrir el archivo\n");
    }else {
        //Creamos un centinela, el cual tendrá una serie de parámetros, entre los cuales se encuentra el doble buffer
        cent = (centinela *)malloc(sizeof(centinela));
        cent->buffer = (char *)calloc(tamBloque*2+2,sizeof(char));
        cent->inicio = 0;
        cent->final = 0;
        cent->lado = IZQ;
        cent->ultimaCarga = IZQ;
        cent->numLineas = 1;
        cent->tamMaxBloque = tamBloque;
        cent->ignore = False;

        // Se obtiene el bloque y se coloca en el primer buffer del centinela
        char * bloque = getBloque(cent,*fd);

        strcpy(cent->buffer,bloque);

        // Indicamos el final de cada buffer
        cent->buffer[tamBloque] = '$';
        cent->buffer[tamBloque*2+1] = '$';
        free(bloque);
    }
    return cent;
}

// Función para obtener un nuevo bloque de caracteres
char * getBloque(centinela * cent, int fd){
    char * bloque = (char *)calloc(cent->tamMaxBloque+1,sizeof(char));
    read(fd,bloque,cent->tamMaxBloque);
    if(strlen(bloque)<cent->tamMaxBloque){
        bloque[strlen(bloque)] = '$';
    }
    return bloque;
}

// Función para solicitar un nuevo caracter
char siguienteCaracter(centinela * cent, int fd){
    char caracter = cent->buffer[cent->final];
    if(caracter == '$'){
        // Si se encuentra en el final del bffer se actualizará el lado que haya sido cargado antes
        if(cent->final == cent->tamMaxBloque || cent->final == (cent->tamMaxBloque*2+1)){
            actualizarBloqueLado(cent,fd);
            if(cent->final == cent->tamMaxBloque){
                cent->final++;
            }else if(cent->final == (cent->tamMaxBloque*2+1)){
                cent->final = 0;
            }
            caracter = cent->buffer[cent->final];
            cent->final++;
        }
    }else{
        cent->final++;
    }
    int tam;
    // Si el tamaño del lexema es demasiado grande recibiremos un error.
    /* En esta práctica, en los comentarios con comillas dobles, así como las cadenas de caracteres,
    en caso de que sobrepasen el tamaño máximo del lexema, se cargará unicamente la parte inicial*/
    if(!cent->ignore && cent->lado == AMBOS){
        if(cent->inicio<cent->final) {
            tam = cent->final-cent->inicio-1;
        }
        else{
            tam = 2 * cent->tamMaxBloque - cent->inicio + cent->final + 1;
        }
        // Lexema demasiado largo
        if(tam > cent->tamMaxBloque){
            cent->ignore = True;
            nuevoError(cent->numLineas,2,"",'\0');
        }
    }
    return caracter;
}

// Función que limpia unicamente un lado determinado del doble buffer (se inicializa a '\0')
void limpiarLadoBuffer(int lado, centinela * cent){
    int from = 0;
    int to = 0;
    switch (lado){
        case IZQ:
            from = 0;
            to = cent->tamMaxBloque;
            break;
        case DER:
            from = cent->tamMaxBloque+1;
            to = cent->tamMaxBloque*2+1;
            break;
    }
    for(int i=from; i<to; i++){
        cent->buffer[i] = '\0';
    }
}
// Función que carga el bloque de caracteres obtenidos, en el buffer correspondiente
void actualizarBloqueLado(centinela * cent, int fd){
    char * buffer = getBloque(cent,fd);
    // Lexema demasiado largo
    if(cent->lado == AMBOS && !cent->ignore){
        cent->ignore = True;
        nuevoError(cent->numLineas,2,"",'\0');
    }

    if(buffer != NULL){
        switch (cent->ultimaCarga){
            case IZQ:
                limpiarLadoBuffer(DER,cent);
                strncpy(&cent->buffer[cent->tamMaxBloque+1],buffer,cent->tamMaxBloque);
                cent->ultimaCarga = DER;
                break;
            case DER:
                limpiarLadoBuffer(IZQ,cent);
                strncpy(&cent->buffer[0],buffer,cent->tamMaxBloque);
                cent->ultimaCarga = IZQ;
                break;
        }
        cent->buffer[cent->tamMaxBloque] = '$';
        cent->buffer[cent->tamMaxBloque*2+1] = '$';
        // Al haber cargado un nuevo bloque, se indica que se está trabajando en ambos buffers
        cent->lado = AMBOS;
    }

    free(buffer);
}

// Función que vuelve al útimo caracter analizado
void devolverCaracter(centinela * cent){
    if(cent->final == cent->tamMaxBloque +1){
        cent->final = cent->tamMaxBloque-1;
    }else if(cent->final == 0){
        cent->final = cent->tamMaxBloque*2;
    }else{
        cent->final--;
    }
}

// Función que actualiza la posición de inicio en el centinela y se actualiza el lado en el que se está trabajanod
void empezarLexema(centinela * cent){
    cent->inicio = cent->final;
    cent->ignore = False;
    if(cent->inicio < cent->tamMaxBloque){
        cent->lado = IZQ;
    }else{
        cent->lado = DER;
    }
}

