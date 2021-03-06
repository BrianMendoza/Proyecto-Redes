/* 
 * File:   centro.c
 * Grupo 33
 * Diana Vainberg 07-41618
 * Brian-Marcial Mendoza 07-41206
 *
 * Created on January 26, 2013, 11:16 PM
 * 
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
#include <pthread.h>
#include <stdbool.h>


/*
 * Variables globales.
 */
int inventario = 0;
int tiempo = 0;
int puerto = 0;
int count = 0;
bool chequeo = false;
FILE *file;
pthread_mutex_t miMutex;


/*
 * Metodo donde se hace el chequeo de los parametros de entrada. 
 * cp es la capacidad maxima, i es el inventario, t es el tiempo 
 * de respuesta, s es la tasa de suministro de la refineria, pt 
 * es el puerto del centro. 
 */
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

/*
 * Metodo que permite realizar la conexion entre un centro y una bomba
 * mediante el uso de sockets. Se utiliza un mutex para evitar que dos
 * hilos que manejan distintas bombas modifiquen variables globales al
 * mismo tiempo. El hilo maneja pedidos de tiempo y de gasolina.
 */
void *conexion(void *socketfd) {
    int socket = *(int*)socketfd;
    int opc,tmp,wr,rd;
    
    rd = read(socket, &tmp, sizeof(tmp));
    if (rd < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }    
    opc = ntohs(tmp);
    
    //Si la opcion es 1, la bomba pidio el tiempo de respuesta, si es 2 pidio
    //una gandola con gasolina.
    if (opc == 1) {
        tmp = htons(tiempo);
        wr = write(socket,&tmp,sizeof(tmp));
        if (wr < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }
        //Cierre seguro del socket para garantizar que no haya mas informacion
        //en el
        shutdown(socket,1);
        while (rd = read(socket, &tmp, sizeof(tmp)) > 0) {}
        close(socket);
    }
    
    if (opc == 2) {
        //Si tiene inventario la respuesta a la bomba es 1, si no hay inventario
        //la respuesta es 2
        bool ok;
        pthread_mutex_lock (&miMutex);
        if (inventario >= 38000)
            ok = true;
        else
            ok = false;
        
        if (ok == true) {
            tmp = htons(1);
            wr = write(socket,&tmp,sizeof(tmp));
            if (wr < 0) {
                perror("ERROR writing to socket");
                exit(1);
            }
            inventario = inventario - 38000;
            if (inventario == 0)
              fprintf(file,"Tanque vacio: minuto %d\n\n",count);  
            chequeo = false;
        }
        else {
            tmp = htons(2);
            wr = write(socket,&tmp,sizeof(tmp));
            if (wr < 0) {
                perror("ERROR writing to socket");
                exit(1);
            }
        }
        
        //Obtiene el nombre de la bomba que hizo la peticion
        char cliente[40];
        bzero(cliente,40);
        rd = read(socket, cliente, 40);
        if (rd < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }
        fprintf(file,"Suministro: %d minutos, %s, %s, %d litros\n\n",count,
                             cliente,(ok) ? "OK" : "Sin Inventario",inventario);
        pthread_mutex_unlock (&miMutex);
        close(socket);
    }
    
    free(socketfd);

}
/*
 * Metodo en el que se crea un socket que escucha las peticiones de
 * tiempo y de gasolina de las bombas, y por cada peticion, crea un
 * hilo, que va a crear un nuevo socket, para conectarse con la bomba
 * que haya hecho el pedido y manejar su peticion.
 */
void *manejarConexiones(void *param) {
    int sockfd,newsockfd,clilen,j;
    struct sockaddr_in serv_addr, cli_addr;
    
    //Setup del socket principal
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(puerto);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR on binding");
      exit(1);
    }
    
    listen(sockfd,10);
    clilen = sizeof(cli_addr);
    while (newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) {
        if (newsockfd < 0) {
          perror("ERROR on accept");
          exit(1);
        }  
        //Se crea el hilo encargado de la peticion de la bomba
        pthread_t encargado;
        int *new_sock;
        new_sock = malloc(1);
        *new_sock = newsockfd;
        
        if( pthread_create( &encargado, NULL,  conexion, (void*) new_sock) < 0) {
            perror("ERROR on handler thread");
            return;
        }
    }
}

/*
 * Este metodo se utiliza para realizar la simulacion de 480 "minutos" 
 * en la que se manejan las conexiones con las bombas que hagan pedidos, 
 * y se escriben los eventos relevantes en el archivo de log. n es el 
 * nombre del centro, cp es la capacidad maxima, s es la tasa de suministro 
 * de la refineria.
 */
void iniciarSimulacion(char *n,int cp, int s) {
    char str[BUFSIZ];
    sprintf(str,"log_%s.txt",n);
    file = fopen(str,"w");
    //Se crea el hilo manejador de conexiones
    pthread_t manejadorConexiones;
    if (pthread_create( &manejadorConexiones, NULL, 
                                manejarConexiones, (void *)NULL) < 0) {
        perror("Error al crear hilo de conexiones");
        exit(1);
    }
    
    if (file != NULL) {
        fprintf(file,"Inventario inicial: %d litros\n\n",inventario);
        while(count < 480) {
            pthread_mutex_lock (&miMutex);
            if (inventario+s < cp)
                inventario = inventario + s;
            else {
                inventario = cp;
                if (chequeo == false){
                        fprintf(file,"Tanque full: %d minutos\n\n",count);
                        chequeo = true;
                }
            }
            ++count;
            pthread_mutex_unlock (&miMutex);
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
 * Funcion principal que toma los parametros de entrada, llama a la funcion
 * que realiza el chequeo de los mismos y comienza la simulacion. 
 */
int main(int argc, char** argv) {
        if ( argc != 13) {
                	printf("Usage: Numero de argumentos invalidos\n");
                	exit(1);
        }
        char *nombre;
        int capacidadMax = 0;
        int suministro = 0;
        
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
        pthread_mutex_init(&miMutex, NULL);
        iniciarSimulacion(nombre,capacidadMax,suministro);
        
    return (EXIT_SUCCESS);
}

