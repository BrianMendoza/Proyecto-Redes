/* 
 * File:   bomba.c
 * Author: brian_m
 *
 * Created on January 26, 2013, 10:09 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <limits.h>
#include <stdbool.h>


typedef struct {
    char nombre[40];
    char direccion[40];
    int puerto;
    int tiempoResp;
} Nodo;

void checkEntrada(int *cp,int *i,int *c) {
    char temp[2*sizeof(long)];
    char *p;
    
    while (38000 > *cp || *cp > 380000) {
        printf("Usage: La capacidad máxima debe ser un valor"
                        " entre 38000 y 380000. Intente de nuevo: ");
        fgets(temp,sizeof(temp),stdin);
        if((p = strchr(temp, '\n')) != NULL)
                *p = '\0';
        *cp = atoi(temp);
    }
    
    while (0 > *i || *i > *cp) {
        printf("Usage: el inventario debe ser un valor"
                        " entre 0 y %d. Intente de nuevo: ",*cp);
        fgets(temp,sizeof(temp),stdin);
        if((p = strchr(temp, '\n')) != NULL)
                *p = '\0';
        *i = atoi(temp);
    }
    
    while (0 > *c || *c > 1000) {
        printf("Usage: El consumo debe ser un valor"
                        " entre 0 y 1000. Intente de nuevo: ");
        fgets(temp,sizeof(temp),stdin);
        if((p = strchr(temp, '\n')) != NULL)
                *p = '\0';
        *c = atoi(temp);
    }
    
    return;
}
int contarLineas(FILE* f) {
    int count = 0;
    char linea[BUFSIZ];
    while ( fgets(linea, sizeof linea, f) ) {
        if ( linea[0] != '\n' )
            ++count;
    }
    return count;
}

void obtenerCentros(Nodo lista[], FILE* f) {
    int size = sizeof(lista);
    int i = 0;
    char str[BUFSIZ];
    char * pch;
    while (fgets(str, sizeof(str), f) != NULL) {
        if ( str[0] != '\n' ) {
                pch = strtok (str,"&");
                sprintf(lista[i].nombre,"%s",pch);
                pch = strtok (NULL, "&");
                sprintf(lista[i].direccion,"%s",pch);
                pch = strtok (NULL, "&");
                lista[i].puerto = atoi(pch);
                ++i;
        }
    }
    return;
}

bool obtenerTiempos(Nodo lista[],int num) {
    int i = 0;
    int counter = 0;

    while(i < num) {
        int sockfd, rd;
        struct sockaddr_in serv_addr;
        struct hostent *server;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("ERROR opening socket");
            exit(1);
        }
        server = gethostbyname(lista[i].direccion);
        if (server == NULL) {
            fprintf(stderr,"ERROR, no such host");
            exit(0);
        }
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,
              (char *)&serv_addr.sin_addr.s_addr,
                                    server->h_length);
        serv_addr.sin_port = htons(lista[i].puerto);
    
        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
            if (lista[i].tiempoResp == -1)
                lista[i].tiempoResp = INT_MAX;
        }
        else {
            int buf,wr;
            buf = htons(1);
            wr = write(sockfd,&buf,sizeof(buf));
            if (wr < 0) {
                perror("ERROR writing to socket");
                exit(1);
            }
            rd = read(sockfd,&buf,sizeof(buf));
            lista[i].tiempoResp = ntohs(buf);
            close(sockfd);
            ++counter;
        }
        ++i;
    }
    if (counter != num -1)
        return true;
    else
        return false;
}

int compararNodos(const void *e1, const void *e2) {

    Nodo *n1 = (Nodo *)e1;
    Nodo *n2 = (Nodo *)e2;
    return ( n1->tiempoResp - n2->tiempoResp );
}

int pedirGasolina(Nodo lista[], int num, char *name, FILE *file, int count) {
    int i = 0;
    int res;

    while(i < num) {
        if (lista[i].tiempoResp == INT_MAX)
            return -1;
        int sockfd, rd;
        struct sockaddr_in serv_addr;
        struct hostent *server;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("ERROR opening socket");
            exit(1);
        }
        server = gethostbyname(lista[i].direccion);
        if (server == NULL) {
            fprintf(stderr,"ERROR, no such host");
            exit(0);
        }
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,
              (char *)&serv_addr.sin_addr.s_addr,
                                    server->h_length);
        serv_addr.sin_port = htons(lista[i].puerto);
        
        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
            fprintf(file,"Peticion: %d minutos, %s, Sin Respuesta\n\n",count,lista[i].nombre);
            ++i;
            continue;
        }
        else {
            int buf,wr;
            buf = htons(2);
            wr = write(sockfd,&buf,sizeof(buf));
            if (wr < 0) {
                perror("ERROR writing to socket");
                exit(1);
            }
            rd = read(sockfd,&buf,sizeof(buf));
            res = ntohs(buf);
            char str[40];
            bzero(str, 40);
            sprintf(str,"%s",name);
            wr = write(sockfd,str,strlen(str));
            shutdown(sockfd,1);
            while (rd = read(sockfd, &buf, sizeof(buf)) > 0) {}
            close(sockfd);
            if (res == 1) {
                fprintf(file,"Peticion: %d minutos, %s, OK\n\n",count,lista[i].nombre);
                return lista[i].tiempoResp;
            }
            else {
                fprintf(file,"Peticion: %d minutos, %s, Sin Inventario\n\n",count,lista[i].nombre);
            }
        }
        ++i;
    }
    return -1;
}

void iniciarSimulacion(char *n,int cp,int i,int c,Nodo lista[],int num) {
    int count = 0;
    FILE *file;
    char str[BUFSIZ];
    sprintf(str,"log_%s.txt",n);
    file = fopen(str,"w");
    bool faltan_tiempos = false;
    bool peticion_hecha = false;
    faltan_tiempos = obtenerTiempos(lista,num);

    qsort((void *) lista, num, sizeof(Nodo), compararNodos);
    

    int capac_ocio = cp - i;
    int cons_neces = 38000 - capac_ocio;
    int tiempo_mas_rapido = lista[0].tiempoResp;
    int tiempo_a_espacio = (cons_neces / c) + 1;
    int tiempo_a_peticion = tiempo_a_espacio - tiempo_mas_rapido;
    if (tiempo_a_peticion < 0)
        tiempo_a_peticion = count;

    int tiempo_gandola = -1;
    bool chequeo = false;
    
    if (file != NULL) {
        fprintf(file,"Inventario inicial: %d litros\n\n",i);
        while(count < 480) {
            if (count == tiempo_gandola) {
                fprintf(file,"Llegada de gandola: %d minutos, 38000 litros\n\n",count);
                i = i + 38000;
                if (i == cp)
                    fprintf(file,"Tanque full: %d minutos\n\n",count);
                chequeo = false;
                peticion_hecha = false;

                capac_ocio = cp - i;
                cons_neces = 38000 - capac_ocio;
                tiempo_a_espacio = count + (cons_neces / c) + 1;
                tiempo_mas_rapido = lista[0].tiempoResp;
                tiempo_a_peticion = tiempo_a_espacio - tiempo_mas_rapido;
                if (tiempo_a_peticion < 0)
                    tiempo_a_peticion = count;

                tiempo_gandola = -1;
            }
            if (count == tiempo_a_peticion /*cp-i >= 38000 && !peticion_hecha*/) {
                if (faltan_tiempos) {
                    faltan_tiempos = obtenerTiempos(lista,num);
                    qsort((void *) lista, num, sizeof(Nodo), compararNodos);
                }
                tiempo_gandola = pedirGasolina(lista,num,n,file,count);
                if (tiempo_gandola != -1) {
                    tiempo_gandola = tiempo_gandola + count/*timer*/;
                    /*peticion_hecha = true;*/
                }
            }
            if (i-c > 0)
                i = i -c;
            else {
                i = 0;
                if (chequeo == false){
                        fprintf(file,"Tanque vacio: %d minutos\n\n",count);
                        chequeo = true;
                }
            }

            ++count;
            usleep(100*1000);
        }
        fclose(file);
    } else {
        perror("Error al crear el archivo");
        exit(1);
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
        if(strcmp(temp,"-n") == 0)
            nombreBomba = argv[i+1];       
        if(strcmp(temp,"-cp") == 0) 
            capacMax = atoi(argv[i+1]);
        if(strcmp(temp,"-i") == 0)
            inventario = atoi(argv[i+1]);         
        if(strcmp(temp,"-c") == 0) 
            consumo = atoi(argv[i+1]);
        if(strcmp(temp,"-fc") == 0) 
            centros = argv[i+1];
        if (strcmp(temp, "-n") != 0 &&
                strcmp(temp, "-cp") != 0 &&
                strcmp(temp, "-i") != 0 &&
                strcmp(temp, "-c") != 0 &&
                strcmp(temp, "-fc") != 0) {
            printf("Usage: Argumento invalido %s\n",temp);
            exit(1);
        }
        i = i + 2;
    }
    
        checkEntrada(&capacMax,&inventario,&consumo);
        
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
    
    int j;
    
    for (j = 0; j < numCentros; j++) {
        listaCentros[j].tiempoResp = -1;
    }
    
    iniciarSimulacion(nombreBomba,capacMax,inventario,
                        consumo,listaCentros,numCentros);
    
    return (EXIT_SUCCESS);
}
