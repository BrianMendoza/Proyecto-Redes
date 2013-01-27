/* 
 * File:   centro.c
 * Author: brian_m
 *
 * Created on January 26, 2013, 11:16 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

/*
 * test
 */
int main(int argc, char** argv) {
        if ( argc != 13) {
                	printf("Usage: Numero de argumentos invalidos\n");
                	exit(1);
                }
        char *nombre;
        int capacidadMax = 0;
        int inventario = 0;
        int tiempo = 0;
        int suministro = 0;
        int puerto = 0;
        
        int i = 1;
        char *temp;
        int temp2 = 0;
        
        while (i < 13) {
            temp = argv[i];
            if (strcmp(temp, "-n") == 0) {
                nombre = argv[i+1];
            }
            if (strcmp(temp, "-cp") == 0) {
                temp2 = atoi(argv[i+1]);
                if (38000 <= temp2 && temp2 <= 3800000)
                        capacidadMax = temp2;
                else {
                    printf("Usage: Capacidad invalida\n");
                    exit(1);
                }
            }
            if (strcmp(temp, "-i") == 0) {
                temp2 = atoi(argv[i+1]);
                if (0 <= temp2 && temp2 <= 3800000)
                        inventario = temp2;
                else {
                    printf("Usage: Inventario invalido\n");
                    exit(1);
                }            }
            if (strcmp(temp, "-t") == 0) {
                temp2 = atoi(argv[i+1]);
                if (0 <= temp2 && temp2 <= 180)
                        tiempo = temp2;
                else {
                    printf("Usage: Tiempo invalido\n");
                    exit(1);
                }            }
            if (strcmp(temp, "-s") == 0) {
                temp2 = atoi(argv[i+1]);
                if (0 <= temp2 && temp2 <= 10000)
                        suministro = temp2;
                else {
                    printf("Usage: Suministro invalido\n");
                    exit(1);
                }            }
            if (strcmp(temp, "-p") == 0) {
                temp2 = atoi(argv[i+1]);
                if (1 <= temp2 && temp2 <= 65535)
                        puerto = temp2;
                else {
                    printf("Usage: Puerto invalido\n");
                    exit(1);
                }            }
            if (strcmp(temp, "-n") != 0 &&
                    strcmp(temp, "-cp") != 0 &&
                    strcmp(temp, "-i") != 0 &&
                    strcmp(temp, "-t") != 0 &&
                    strcmp(temp, "-s") != 0 &&
                    strcmp(temp, "-p") != 0) {
                printf("Usage: Argumento invalido\n");
		exit(1);
            }
            i = i+2;
        }
        
        while (capacidadMax < inventario) {
        	printf("Inventario excede capacidad, intente de nuevo: ");
		char temp3[sizeof(int)];
		char *p;
		fgets(temp3, sizeof(temp3), stdin);
		if ((p = strchr(temp3, '\n')) != NULL)
		{*p = '\0';}
		inventario = atoi(temp3);
        }
        
        printf("%s %d %d %d %d %d\n",nombre,capacidadMax,inventario,tiempo,suministro,puerto);
    return (EXIT_SUCCESS);
}

