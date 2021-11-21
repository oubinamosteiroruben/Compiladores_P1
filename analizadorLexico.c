
#include "analizadorLexico.h"
#include "sistemaEntrada.h"
#include "abb.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "definiciones.h"
#include "tablaSimbolos.h"
#include "gestorErrores.h"

// Estados para los distintos tipos de números (no decimales)
#define E_ 2
#define EBXO 3

#define TIPO_B 2
#define TIPO_X 16
#define TIPO_O 8

// Estados para los números decimales
#define ED 1
#define ED_ 2
#define EP 3
#define EE 4
#define EPD 5
#define EPD_ 6
#define EES 7
#define EED 8
#define EED_ 9
#define EF_ENTERO 10
#define EF_FLOAT 11
#define EF_EXP 12

void analizadorGeneral(tipoelem * E, centinela * cent, int fd);
void formarCL_id(tipoelem * E, centinela * cent,int fd);
void formarCL_cadena_coment(tipoelem * E, centinela * cent, int fd);
void formarCL_coment(tipoelem  * E,centinela * cent,int fd,int tam);
void formarCL_String_Doble(tipoelem * E,centinela * cent,int fd,int tam);
void formarCL_String_Simple(tipoelem * E,centinela * cent,int fd,int tam);
int isDelimitador_Operador(char caracter);
void formarCL_Delimitador_Operador(tipoelem * E,centinela * cent,int fd,int tam);
void formarCL_num(tipoelem * E, centinela * cent,int fd,int tam);
void formarCL_numBXO(tipoelem * E,centinela * cent,int fd,int tam,int tipo, int estado);
int isTipo(char caracter, int tipo);
void formarCL_numDec(tipoelem * E,centinela * cent,int fd,int tam,int estado);

// Función que carga devuelve un nuevo lexema
tipoelem  siguienteLexema(centinela * cent, int fd, abb * tablaSimbolos){
    tipoelem  E;

    E.componenteLexico = -1;
    E.encontrado = False;
    E.eof = False;

    char caracterActual;
    // Se realizará una búsqueda de caracteres hasta encontrar un lexema
    do{
        // Se reserva memoria para el lexema y se inician todos los caracteres a '\0'
        E.lexema = (char *)calloc(cent->tamMaxBloque+1,sizeof(char));
        // Obtiene el primer caracter y lo analiza
        caracterActual = siguienteCaracter(cent,fd);
        E.lexema[0] = caracterActual;
        analizadorGeneral(&E,cent,fd);
        // Si no se encontró un componente léxico, se liberan los caracteres obtenidos
        if(!E.encontrado){
            free(E.lexema);
        }
        // Si no se encontró un componente léxico / el final del archivo, se vuelve a realizar una bsúqueda
    }while(E.eof == False && !E.encontrado);

    // Se reajusta el tamaño del buffer done está el lexema
    E.lexema = (char *)realloc(E.lexema,strlen(E.lexema)+1);

    // Si se trata de un identificador y no está en la tabla, se insertará una copia de este
    if(E.componenteLexico == CL_ID ){
        tipoelem EAux;
        if(es_miembro(*tablaSimbolos,E)){
            buscar_nodo(*tablaSimbolos,E.lexema,&EAux);
            E.componenteLexico = EAux.componenteLexico;
        }else {
            EAux.lexema = (char*)malloc(sizeof(char)*(strlen(E.lexema)+1));
            strcpy(EAux.lexema,E.lexema);
            EAux.componenteLexico = E.componenteLexico;
            insertarPalabra(EAux,tablaSimbolos);
        }
    }
    return E;
}

// Función que analizará el primer caracter del lexema con el fin de redirigirlo a un autómata
void analizadorGeneral(tipoelem * E, centinela * cent, int fd){
    char caracter = E->lexema[0];
    switch (caracter){
        case '\t':
            E->encontrado = True;
            E->componenteLexico = CL_TAB;
            empezarLexema(cent);
            break;
        case ' ':
            E->encontrado = False;
            E->lexema[0] = '\n';
            E->componenteLexico = -1;
            empezarLexema(cent);
            break;
        case '\n' :
            E->encontrado = True;
            E->lexema[0] = '\n';
            E->componenteLexico = CL_SALTO;
            cent->numLineas++;
            empezarLexema(cent);
            break;
        case '$':
            E->encontrado = True;
            E ->lexema[0] = EOF;
            E->eof = True;
            break;
        case '#':
            E->encontrado = True;
            cent->ignore = True;
            while(caracter!='\n'){
                caracter = siguienteCaracter(cent,fd);
            }
            devolverCaracter(cent);
            E->lexema[0] = caracter;
            E->componenteLexico = -1;
            empezarLexema(cent);
            break;
        case '\"':
            E->encontrado = True;
            formarCL_cadena_coment(E,cent,fd);
            break;
        case '\'':
            E->encontrado = True;
            E->componenteLexico = CL_CADENA_SIMPLE;
            formarCL_String_Simple(E,cent,fd,1);
            break;
        default:
            if(isalpha(E->lexema[0]) || E->lexema[0] == '_'){
                E->encontrado = True;
                E->componenteLexico = CL_ID;
                formarCL_id(E,cent,fd); // AUTOMATA IDENTIFICADORES
            }else if(isDelimitador_Operador(caracter)){
                E->encontrado = True;
                if(caracter == '~' || caracter == '(' || caracter == ')' || caracter == '[' || caracter == ']' || caracter == '{' || caracter == '}' || caracter == ',' || caracter == ';'){ // Único operando
                    if(caracter == '~') {
                        E->componenteLexico = CL_OP_SIMPLE;
                    }else{
                        E->componenteLexico = CL_DEL_SIMPLE;
                    }
                    empezarLexema(cent);
                }else{
                    formarCL_Delimitador_Operador(E,cent,fd,1);
                }
            }else if(isdigit(caracter)){ // AUTOMATA NUMEROS
                formarCL_num(E,cent,fd,1);
            }else{
                nuevoError(cent->numLineas,1,E->lexema,caracter);
            }
            break;
    }
}

// AUTOMATA DE IDENTIFICADORES
void formarCL_id(tipoelem * E, centinela * cent,int fd){
    char caracter;
    int flag = 1;
    int tam = 1;
    // Va a buscar nuevos caracteres hasta que se finalice el identificador o el lexema sea demasiado grande
    while(flag && (tam < cent->tamMaxBloque) ){
        caracter = siguienteCaracter(cent,fd);
        tam++;
        if(isalnum(caracter) || caracter == '_'){
            E->lexema[tam-1] = caracter;
        }else{
            E->lexema[tam-1] = '\0';
            flag = 0;
        }
    }
    // Cuando se finalice el identificador correctamente, se volverá a analizar el caracter actual con el fin de redirigirlo a otro autómata
    if((tam < cent->tamMaxBloque)){
        devolverCaracter(cent);
    }else{
        // Si el tamano del lexema excede los límites se muestra un error
        nuevoError(cent->numLineas,2,E->lexema,'\0');
    }
    empezarLexema(cent);
}


// AUTOMATA QUE REDIRGIRÁ AL AUTÓMATA DE CADENAS CON COMILLAS DOBLES O AL DE COMENTARIOS CON COMILLAS
void formarCL_cadena_coment(tipoelem * E, centinela * cent, int fd){
    char caracter;
    int tam = 1;
    caracter = siguienteCaracter(cent,fd);
    tam++;
    E->lexema[tam-1] = caracter;
    if(caracter == '\"'){
        caracter = siguienteCaracter(cent,fd);
        if(caracter == '\"'){ // ES COMENTARIO
            tam++;
            E->lexema[tam-1] = caracter;
            E->componenteLexico = CL_COMENTARIO;
            formarCL_coment(E,cent,fd,tam); // ENTRA EN EL AUTOMATA DE COMENTARIO CON COMILLAS
        }else{ // ES FIN DE STRING
            E->componenteLexico = CL_CADENA_DOBLE;
            devolverCaracter(cent);
            empezarLexema(cent);
        }
    }else{
        if(caracter == '\n') cent->numLineas++;
        formarCL_String_Doble(E,cent,fd,tam); // ENTRA EN EL AUTOMATA DE STRINGS CON COMILLAS DOBLES
        E->componenteLexico = CL_CADENA_DOBLE;
    }

}

// AUTOMATA DE COMENTARIOS CON COMILLAS

/*En caso de que el tamaño del comentario sea demasiado grande, se ignorarán los nuevos caracteres (no se insertarán
 * en el lexema pero si se analizarán). */

void formarCL_coment(tipoelem  * E,centinela * cent,int fd, int tam){
    char caracter = siguienteCaracter(cent,fd);
    tam++;
    if(!cent->ignore) E->lexema[tam-1] = caracter;
    if(caracter == '\"'){
        caracter = siguienteCaracter(cent,fd);
        tam++;
        if(!cent->ignore) E->lexema[tam-1] = caracter;
        if(caracter == '\"'){
            caracter = siguienteCaracter(cent,fd);
            tam++;
            if(!cent->ignore)E->lexema[tam-1] = caracter;
            if(caracter == '\"'){
                empezarLexema(cent);
            }else if(caracter != '$'){
                if(caracter == '\n') cent->numLineas++;
                formarCL_coment(E,cent,fd,tam);
            }else{
                nuevoError(cent->numLineas,0,E->lexema,'\0');
                E->lexema[0] = EOF;
                E->eof = True;
            }
        }else if(caracter != '$'){
            if(caracter == '\n') cent->numLineas++;
            formarCL_coment(E,cent,fd,tam);
        }else{
            nuevoError(cent->numLineas,0,E->lexema,'\0');
            E->lexema[0] = EOF;
            E->eof = True;
        }
    }else if(caracter != '$'){
        if(caracter == '\n') cent->numLineas++;
        formarCL_coment(E,cent,fd,tam);
    }else{
        nuevoError(cent->numLineas,0,E->lexema,'\0');
        E->lexema[0] = EOF;
        E->eof = True;
    }
}

// AUTOMATA DE COMENTARIO DE COMILLAS DOBLES
/*En caso de que el tamaño del comentario sea demasiado grande, se ignorarán los nuevos caracteres (no se insertarán
 * en el lexema pero si se analizarán). */

void formarCL_String_Doble(tipoelem * E,centinela * cent,int fd,int tam){
    char caracter = siguienteCaracter(cent,fd);
    tam++;
    if(!cent->ignore) E->lexema[tam-1] = caracter;
    if(caracter == '\"'){ // Finaliza correctamente
        empezarLexema(cent);
    }else if(caracter == '$'){ // Final inesperado
        nuevoError(cent->numLineas,0,E->lexema,'\0');
        E->lexema[0] = EOF;
        E->eof = True;
    }else{
        formarCL_String_Doble(E,cent,fd,tam); // Continua en el autómata
    }
}

// AUTOMATA DE STRING SIMPLE
/*En caso de que el tamaño del comentario sea demasiado grande, se ignorarán los nuevos caracteres (no se insertarán
 * en el lexema pero si se analizarán). */

void formarCL_String_Simple(tipoelem * E,centinela * cent,int fd,int tam){
    char caracter = siguienteCaracter(cent,fd);
    tam++;
    if(caracter == '\n') cent->numLineas++;
    if(!cent->ignore) E->lexema[tam-1] = caracter;
    if(caracter == '\'') { // Finaliza correctamente
        empezarLexema(cent);
    }else if(caracter == '$') { // Final inesperado
        nuevoError(cent->numLineas,0,E->lexema,'\0');
        E->lexema[0] = EOF;
        E->eof = True;
    }else{
        formarCL_String_Simple(E,cent,fd,tam); // Continua en el autómata
    }
}

// Función que comprueba si el caracter indicado puede tratarse de un delimitador o un operador
int isDelimitador_Operador(char caracter){
    if(caracter == 33 || caracter == 37 || caracter == 38 || (caracter >= 40 && caracter <= 47) || (caracter >= 58 && caracter <= 62) || caracter == 64 || caracter == 91 || caracter == 93 || caracter == 94 || (caracter >= 123 && caracter <= 126)){
        return True;
    }
    return False;
}

// AUTOMATA DE DELIMITADORES
void formarCL_Delimitador_Operador(tipoelem * E,centinela * cent,int fd,int tam){
    char caracter = siguienteCaracter(cent,fd);
    tam++;
    switch (E->lexema[0]){
        case '+': case '@': case '%': case '|': case '&': case '^':
            if(caracter == '='){
                E->lexema = realloc(E->lexema,(tam+1)*sizeof(char));
                E->lexema[tam-1] = caracter;
                E->componenteLexico = CL_DEL_DOBLE;
                empezarLexema(cent);
            }else{
                E->componenteLexico = CL_DEL_SIMPLE;
                devolverCaracter(cent);
                empezarLexema(cent);
            }
            break;
        case '-':
            if(caracter == '=' || caracter == '>'){
                E->lexema[tam-1] = caracter;
                E->componenteLexico = CL_DEL_DOBLE;
                empezarLexema(cent);
            }else{
                E->componenteLexico = CL_OP_SIMPLE;
                devolverCaracter(cent);
                empezarLexema(cent);
            }
            break;
        case '*':
            if(caracter == '='){
                E->lexema[tam-1] = caracter;
                E->componenteLexico = CL_DEL_DOBLE;
                empezarLexema(cent);
            }else if(caracter == '*') {
                E->lexema[tam-1] = caracter;
                caracter = siguienteCaracter(cent,fd);
                tam++;
                if(caracter == '=') {
                    E->lexema[tam-1] = caracter;
                    E->componenteLexico = CL_DEL_TRIPLE;
                    empezarLexema(cent);
                }else {
                    E->componenteLexico = CL_OP_DOBLE;
                    devolverCaracter(cent);
                    empezarLexema(cent);
                }
            }else{
                E->componenteLexico = CL_OP_SIMPLE;
                devolverCaracter(cent);
                empezarLexema(cent);
            }
            break;
        case '/':
            if(caracter == '/'){
                E->lexema[tam-1] = caracter;
                caracter = siguienteCaracter(cent,fd);
                tam++;
                if(caracter == '='){
                    E->lexema[tam-1] = caracter;
                    E->componenteLexico = CL_DEL_TRIPLE;
                    empezarLexema(cent);
                }else{
                    devolverCaracter(cent);
                    E->componenteLexico = CL_OP_DOBLE;
                    empezarLexema(cent);
                }
            }else if(caracter == '='){
                E->lexema[tam-1] = caracter;
                E->componenteLexico = CL_DEL_DOBLE;
                empezarLexema(cent);
            }else{
                E->componenteLexico = CL_DEL_SIMPLE;
                devolverCaracter(cent);
                empezarLexema(cent);
            }
            break;
        case '<': case '>':
            if(caracter == E->lexema[0]) {
                E->lexema[tam-1] = caracter;
                caracter = siguienteCaracter(cent,fd);
                tam++;
                if(caracter == '='){
                    E->lexema[tam-1] = caracter;
                    E->componenteLexico = CL_DEL_TRIPLE;
                    empezarLexema(cent);
                }else{
                    E->componenteLexico = CL_OP_DOBLE;
                    devolverCaracter(cent);
                    empezarLexema(cent);
                }
            }else if(caracter == '='){
                E->lexema[tam-1] = caracter;
                E->componenteLexico = CL_OP_DOBLE;
                empezarLexema(cent);
            }else{
                E->componenteLexico = CL_OP_SIMPLE;
                devolverCaracter(cent);
                empezarLexema(cent);
            }
            break;
        case ':': case '=':
            if(caracter == '='){
                E->lexema[tam-1] = caracter;
                E->componenteLexico = CL_OP_DOBLE;
                empezarLexema(cent);
            }else{
                E->componenteLexico = CL_DEL_SIMPLE;
                devolverCaracter(cent);
                empezarLexema(cent);
            }
            break;
        case '!':
            if(caracter == '='){
                E->lexema[tam-1] = caracter;
                E->componenteLexico = CL_OP_DOBLE;
                empezarLexema(cent);
            }else{
                nuevoError(cent->numLineas,1,E->lexema,caracter);
                E->lexema[0] = '\n';
                E->encontrado = False;
            }
            break;
        case '.':
            if(isdigit(caracter)){
                E->lexema[tam-1] = caracter;
                formarCL_numDec(E,cent,fd,tam,EPD);
            }else{
                E->componenteLexico = CL_DEL_SIMPLE;
                devolverCaracter(cent);
                empezarLexema(cent);
            }
            break;
        default:
            break;
    }
}

// AUTOMATA DE NUMEROS
void formarCL_num(tipoelem * E, centinela * cent,int fd,int tam){
    char caracter = siguienteCaracter(cent,fd);
    E->encontrado = True;
    if(E->lexema[0] == '0'){
        switch (caracter){
            case 'b': case 'B': // BINARIO
                tam++;
                E->componenteLexico = CL_NUM_BINARIO;
                E->lexema[tam-1] = caracter;
                formarCL_numBXO(E,cent,fd,tam,TIPO_B,E_);
                break;
            case 'x': case 'X': // HEXADECIMAL
                tam++;
                E->componenteLexico = CL_NUM_HEXA;
                E->lexema[tam-1] = caracter;
                formarCL_numBXO(E,cent,fd,tam,TIPO_X,E_);
                break;
            case 'o': case 'O': // OCT
                tam++;
                E->componenteLexico = CL_NUM_OCT;
                E->lexema[tam-1] = caracter;
                formarCL_numBXO(E,cent,fd,tam,TIPO_O,E_);
                break;
            case '.': // DECIMAL EN ESTADO DEL PUNTO FLOTANTE
                tam++;
                E->lexema[tam-1] = caracter;
                formarCL_numDec(E,cent,fd,tam,EP);
                break;
            case 'e': // DECIMAL EN ESTADO EXPONENCIAL
                tam++;
                E->lexema[tam-1] = caracter;
                formarCL_numDec(E,cent,fd,tam,EE);
                break;
            default: // DECIMAL ENTERO (0)
                E->componenteLexico = CL_NUM_ENTERO;
                if(!isDelimitador_Operador(caracter) && (caracter != '\t' && caracter != '\n' && caracter != ' ')) nuevoError(cent->numLineas,1,E->lexema,'\0');
                devolverCaracter(cent);
                empezarLexema(cent);
                break;
        }
    }else{
        devolverCaracter(cent);
        formarCL_numDec(E,cent,fd,tam,ED); // ENTRA EN EL AUTÓMATA DE NÚMEROS DECIMALES
    }
}

// AUTOMATA DE BINARIOS/HEXA/OCTA
void formarCL_numBXO(tipoelem * E,centinela * cent,int fd,int tam,int tipo, int estado){
    char caracter = siguienteCaracter(cent,fd);
    switch (estado){
        case E_: // ESTADO DESPUÉS DE _ (SOLO ACEPTA UN CARACTER DEL TIPO)
            if(isTipo(caracter,tipo) && tam<cent->tamMaxBloque){
                tam++;
                E->lexema[tam-1] = caracter;
                formarCL_numBXO(E,cent,fd,tam,tipo,EBXO); // CAMBIA DE ESTADO EN EL AUTÓMATA
            }else{
                if(tam<cent->tamMaxBloque){
                    nuevoError(cent->numLineas,1,E->lexema,caracter); // ERROR DE CARACTER NO VALIDO
                }else{
                    nuevoError(cent->numLineas,2,E->lexema,'\0'); // ERROR DE TAMAÑO
                }
                devolverCaracter(cent);
                empezarLexema(cent);
            }
            break;
        case EBXO: // ESTADO DESPUES DE UN CARACTER DEL TIPO
            if(isTipo(caracter,tipo) && tam<cent->tamMaxBloque){
                tam++;
                E->lexema[tam-1] = caracter;
                formarCL_numBXO(E,cent,fd,tam,tipo,EBXO); // SIGUE EN EL MISMO ESTADO DEL AUTÓMATA
            }else if(caracter == '_' && tam<cent->tamMaxBloque){
                tam++;
                E->lexema[tam-1] = caracter;
                formarCL_numBXO(E,cent,fd,tam,tipo,E_); // CAMBIA DE ESTADO EN EL AUTOMATA
            }else if(isDelimitador_Operador(caracter) || caracter == '\n' || caracter == '\t' || caracter == ' ' || caracter == '$') { // FINALIZA EL NÚMERO
                devolverCaracter(cent);
                empezarLexema(cent);
            }else{
                if(tam<cent->tamMaxBloque){
                    nuevoError(cent->numLineas,1,E->lexema,caracter); // ERROR DE CARACTER NO VALIDO
                }else{
                    nuevoError(cent->numLineas,2,E->lexema,'\0'); // ERROR DE TAMAÑO
                }
                devolverCaracter(cent);
                empezarLexema(cent);
            }
            break;
    }
}

// Función que comprueba si un caracter pertenece a un tipo de numero determinado
int isTipo(char caracter, int tipo){
    switch (tipo){
        case TIPO_B: // Binario
            if(caracter >= 48 && caracter <=49){
                return True;
            }
            return False;
        case TIPO_X: // Hexadecimal
            if((caracter >= 48 && caracter <=57)||(caracter >=65 && caracter<=70)||(caracter >=97 && caracter<=102)){
                return True;
            }
            return False;
        case TIPO_O: // Octadecimal
            if(caracter >= 48 && caracter <=55){
                return True;
            }
            return False;
    }
    return False;
}

// AUTOMATA DE NÚMEROS DECIMALES
void formarCL_numDec(tipoelem * E,centinela * cent,int fd,int tam,int estado) {
    char caracter = siguienteCaracter(cent,fd);
    int flagError = False;
    if(tam<cent->tamMaxBloque){
        switch (estado){
            case ED: // Es entero (el último caracter es un digito)
                if(isdigit(caracter)){
                    estado = ED;
                }else if(caracter == '_'){
                    estado = ED_;
                }else if(caracter == '.'){
                    estado = EP;
                }else if(caracter == 'e'){
                    estado = EE;
                }else if(isDelimitador_Operador(caracter)){
                    estado = EF_ENTERO;
                }else{ //
                    estado = EF_ENTERO;
                    if(caracter != '\t' && caracter != '\n' && caracter != ' ') flagError = True; // Final inesperado
                }
                break;
            case ED_: // Es entero (el último caracter es una _ )
                if(isdigit(caracter)){
                    estado = ED;
                }else{
                    estado = EF_ENTERO;
                    flagError = True;
                }
                break;
            case EP: // Es punto flotante (el último caracter es el . )
                if(isdigit(caracter)){
                    estado = EPD;
                }else if(isDelimitador_Operador(caracter)) {
                    estado = EF_FLOAT;
                }else{
                    estado = EF_FLOAT;
                    if(caracter != '\t' && caracter != '\n' && caracter != ' ')flagError = True;
                }
                break;
            case EPD: // Es un punto flotante (el último caracter es un digito)
                if(isdigit(caracter)){
                    estado = EPD;
                }else if(caracter == '_'){
                    estado = EPD_;
                }else if(caracter == 'e'){
                    estado = EE;
                }else if(isDelimitador_Operador(caracter)){
                    estado = EF_FLOAT;
                }else{
                    estado = EF_FLOAT;
                    if(caracter != '\t' && caracter != '\n' && caracter != ' ')flagError = True;
                }
                break;
            case EPD_: // Es un punto flotante (el último caracter es una _ )
                if(isdigit(caracter)){
                    estado = EPD;
                }else{
                    estado = EF_FLOAT;
                    flagError = True;
                }
                break;
            case EE: // Es un exponencial (el último caracter es la e)
                if(isdigit(caracter)){
                    estado = EED;
                }else if(caracter == '+' || caracter == '-'){
                    estado = EES;
                }else{
                    estado = EF_EXP;
                    flagError = True;
                }
                break;
            case EES: // Es un exponencial (el último caracter es + o - )
                if(isdigit(caracter)){
                    estado = EED;
                }else{
                    estado = EF_EXP;
                    flagError = True;
                }
                break;
            case EED: // Es un exponencial (el último caracter es un digito)
                if(isdigit(caracter)){
                    estado = EED;
                }else if(caracter == '_'){
                    estado = EED_;
                }else if(isDelimitador_Operador(caracter)){
                    estado = EF_EXP;
                }else{
                    estado = EF_EXP;
                    if(caracter != '\t' && caracter != '\n' && caracter != ' ')flagError = True;
                }
                break;
            case EED_: // Es un exponencial (el último caracter es una _ )
                if(isdigit(caracter)){
                    estado = EED;
                }else{
                    estado = EF_EXP;
                    flagError = True;
                }
                break;
        }
    }else{ // Si excede el tamaño máximo, finaliza el autómata numérico y guarda error
        nuevoError(cent->numLineas,2,E->lexema,'\0');
        switch (estado){
        case ED: case ED_:
            estado = EF_ENTERO;
            break;
        case EP: case EPD: case EPD_:
            estado = EF_FLOAT;
            break;
        case EES: case EE: case EED: case EED_:
            estado = EF_EXP;
            break;
        }
    }

    // Carga el error en caso de que se produzca
    if(flagError){
        nuevoError(cent->numLineas,1,E->lexema,caracter);
    }

    if(estado != EF_ENTERO && estado != EF_FLOAT && estado != EF_EXP){
        tam++;
        E->lexema[tam-1] = caracter;
        formarCL_numDec(E,cent,fd,tam,estado); // Continua en el mismo autómata
    }else{
        // Indica de que tipo de componente léxico se trata
        switch (estado){
            case EF_ENTERO:
                E->componenteLexico = CL_NUM_ENTERO;
                break;
            case EF_FLOAT:
                E->componenteLexico = CL_NUM_FLOAT;
                break;
            case EF_EXP:
                E->componenteLexico = CL_NUM_EXP;
                break;
        }
        devolverCaracter(cent);
        empezarLexema(cent);
    }
}
