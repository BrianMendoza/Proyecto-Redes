/* 
 * File:   centro.c
 * Author: brian_m
 *
 * Created on January 26, 2013, 11:16 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>


void checkEntrada(int *cp,int *i,int *t,int *s,int *pt) {
    char temp[2*sizeof(long)];
    char *p;
    
    while (38000 > *cp || *cp > 3800000) {
        printf("Usage: La capacidad máxima debe ser un valor"
                        " entre 38000 y 3800000. Intente de nuevo: ");
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
    
    while (0 > *t || *t > 180) {
        printf("Usage: El tiempo debe ser un valor"
                        " entre 0 y 180. Intente de nuevo: ");
        fgets(temp,sizeof(temp),stdin);
        if((p = strchr(temp, '\n')) != NULL)
                *p = '\0';
        *t = atoi(temp);
    }
    
    while (0 > *s || *s > 10000) {
        printf("Usage: El suministro debe ser un valor"
                        " entre 0 y 10000. Intente de nuevo: ");
        fgets(temp,sizeof(temp),stdin);
        if((p = strchr(temp, '\n')) != NULL)
                *p = '\0';
        *s = atoi(temp);
    }
    
    while (0 > *pt || *pt > 65535) {
        printf("Usage: El puerto debe ser un valor"
                        " entre 0 y 65535. Intente de nuevo: ");
        fgets(temp,sizeof(temp),stdin);
        if((p = strchr(temp, '\n')) != NULL)
                *p = '\0';
        *pt = atoi(temp);
    }
    
    return;
}

void iniciarSimulacion(char *n,int cp, int i, int t, int s, int p) {
    int count = 0;
    FILE *file;
    char str[BUFSIZ];
    sprintf(str,"log_%s.txt",n);
    file = fopen(str,"w");

    int sockfd,newsockfd,clilen,j;
    struct sockaddr_in serv_addr, cli_addr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(p);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR on binding");
      exit(1);
    }
    
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
      perror("ERROR on accept");
      exit(1);
    }
    bzero(str,256);

    int temp = htonl(t);
    j = write(newsockfd,&temp,sizeof(temp));
    if (j < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
    close(sockfd);
    close(newsockfd);
    printf("Socket done!\n");
    if (file != NULL) {
        fprintf(file,"Estado inicial: %d\n\n",i);
        while(count < 480) {
            if (i+s <= cp)
                i = i + s;
            else {
                i = cp;
                fprintf(file,"Tanque full: minuto %d\n\n",count);
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
        
        while (i < 13) {
            temp = argv[i];
            if (strcmp(temp, "-n") == 0) 
                nombre = argv[i+1];
            if (strcmp(temp, "-cp") == 0) 
                capacidadMax = atoi(argv[i+1]);
            if (strcmp(temp, "-i") == 0) 
                inventario = atoi(argv[i+1]);
            if (strcmp(temp, "-t") == 0) 
                tiempo = atoi(argv[i+1]);
            if (strcmp(temp, "-s") == 0) 
                suministro = atoi(argv[i+1]);
            if (strcmp(temp, "-p") == 0) 
                puerto = atoi(argv[i+1]);
            if (strcmp(temp, "-n") != 0 &&
                    strcmp(temp, "-cp") != 0 &&
                    strcmp(temp, "-i") != 0 &&
                    strcmp(temp, "-t") != 0 &&
                    strcmp(temp, "-s") != 0 &&
                    strcmp(temp, "-p") != 0) {
                printf("Usage: Argumento invalido %s\n",temp);
		exit(1);
            }
            i = i+2;
        }
        
        checkEntrada(&capacidadMax,&inventario,&tiempo,&suministro,&puerto);
        
        iniciarSimulacion(nombre,capacidadMax,inventario,
                                tiempo,suministro,puerto);
        
    return (EXIT_SUCCESS);
}

