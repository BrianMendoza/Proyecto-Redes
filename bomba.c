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

typedef struct {
    char nombre[BUFSIZ];
    char direccion[BUFSIZ];
    int puerto;
    int tiempoResp;
    int sockfd;
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
                pch = strtok (str,"&&");
                sprintf(lista[i].nombre,"%s",pch);
                pch = strtok (NULL, "&&");
                sprintf(lista[i].direccion,"%s",pch);
                pch = strtok (NULL, "&&");
                lista[i].puerto = atoi(pch);
                ++i;
        }
    }
    return;
}

void obtenerTiempos(Nodo lista[],int num) {
    int i = 0;

    while(i < num) {
        int sockfd, j;
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
            perror("ERROR connecting");
            exit(1);
        }
    
        int tmp,buf;
        j = read(sockfd,&buf,sizeof(buf));
        if (j < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }
        tmp = ntohl(buf);
        lista[i].tiempoResp = tmp;
        printf("Tiempo: %d\n",tmp);
    
        close(sockfd);
        printf("Socket done!\n");
        ++i;
    }
    return;
}

int compararNodos(const void *e1, const void *e2) {

    Nodo *n1 = (Nodo *)e1;
    Nodo *n2 = (Nodo *)e2;
    return ( n1->tiempoResp - n2->tiempoResp );
}

void iniciarSimulacion(char *n,int cp,int i,int c,Nodo lista[],int num) {
    int count = 0;
    FILE *file;
    char str[BUFSIZ];
    sprintf(str,"log_%s.txt",n);
    file = fopen(str,"w");
    
    obtenerTiempos(lista,num);

    qsort((void *) lista, num, sizeof(Nodo), compararNodos);
    
    if (file != NULL) {
        fprintf(file,"Estado inicial: %d\n\n",i);
        while(count < 480) {
            if (i-c >= 0)
                i = i -c;
            else {
                i = 0;
                fprintf(file,"Tanque vacio: minuto %d\n\n",count);
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
    
    iniciarSimulacion(nombreBomba,capacMax,inventario,
                        consumo,listaCentros,numCentros);
    
    return (EXIT_SUCCESS);
}
