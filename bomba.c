/* 
 * File:   bomba.c
 * Author: brian_m
 *
 * Created on January 26, 2013, 10:09 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char nombre[BUFSIZ];
    char direccion[BUFSIZ];
    int puerto;
} Nodo;

int contarLineas(FILE* f) {
    int count = 0;
    char linea[BUFSIZ];
    while ( fgets(linea, sizeof linea, f) ) {
        if ( linea[0] != '\n' )
            ++count;
    }
    printf("%d lines\n",count);
    return count;
}

void obtenerCentros(Nodo lista[], FILE* f) {
    int size = sizeof(lista);
    int i = 0;
    char str[BUFSIZ];
    char * pch;
    while (fgets(str, sizeof(str), f) != NULL) {
        if ( str[0] != '\n' ) {
                pch = strtok (str,"&&");
                strcpy(lista[i].nombre,pch);
                pch = strtok (NULL, "&&");
                strcpy(lista[i].direccion,pch);
                pch = strtok (NULL, "&&");
                lista[i].puerto = atoi(pch);
                ++i;
        }
    }
    return;
}
/*
 * test 
 */
int main(int argc, char** argv) {
    if(argc != 11){
        printf("Usage: Numero de argumentos invalido\n");
        exit(1);
    }
    char *nombreBomba;
    int capacMax;
    int inventario;
    int consumo;
    char *centros;
    int i = 1;
    
    while(i < argc){
        char *temp = argv[i];
        
                if(strcmp(temp,"-n") == 0) {
                    nombreBomba = argv[i+1];
                }
                
                if(strcmp(temp,"-cp") == 0) {
                    capacMax = atoi(argv[i+1]);
                    while((capacMax < 38000) || (capacMax > 380000)){
                        printf("Usage: La capacidad máxima debe ser un valor"
                                " entre 38000 y 380000. Intente de nuevo: ");
                        char temp[sizeof(long)];
                        char *p;
                        fgets(temp,sizeof(temp),stdin);
                        if((p = strchr(temp, '\n')) != NULL){
                            *p = '\0';
                        }
                        capacMax = atoi(temp);
                        }
                }
                
                if(strcmp(temp,"-i") == 0) {
                    inventario = atoi(argv[i+1]);
                }
                
                if(strcmp(temp,"-c") == 0) {
                    consumo = atoi(argv[i+1]);
                    while((consumo < 0) || (consumo > 1000)){
                        printf("Usage: El consumo promedio debe ser un valor"
                                " entre 0 y 1000. Intente de nuevo: ");
                        char temp[sizeof(long)];
                        char *p;
                        fgets(temp,sizeof(temp),stdin);
                        if((p = strchr(temp, '\n')) != NULL){
                            *p = '\0';
                        }
                        consumo = atoi(temp);
                        }
                }
                if(strcmp(temp,"-fc") == 0) {
                    centros = argv[i+1];
                }
        i = i + 2;
    }
    while((inventario < 0) || (inventario > capacMax)){
        printf("Usage: El inventario debe tener un valor entre 0 y la capacidad "
                "máxima. Intente de nuevo: ");
        char temp[sizeof(long)];
        char *p;
        fgets(temp,sizeof(temp),stdin);
        if((p = strchr(temp, '\n')) != NULL){
                *p = '\0';
        }
        inventario = atoi(temp);
    }    
    
    printf("%s,%d,%d,%d,%s\n",nombreBomba,capacMax,inventario, consumo, centros);
    
    FILE *file = NULL;
    file = fopen(centros,"r");
    int numCentros = 0;
    
    if (file != NULL) {
        numCentros = contarLineas(file);
    } else {
        perror("Error al abrir el archivo");
	exit(1);
    }
    
    rewind(file);
    Nodo listaCentros[numCentros];
    obtenerCentros(listaCentros,file);
    fclose(file);
    
    return (EXIT_SUCCESS);
}

