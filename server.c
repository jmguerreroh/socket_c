/**
 * @file server.c
 * @author Jose Miguel Guerrero Hernandez (josemiguel.guerrero@urjc.es)
 * @brief Servidor socket en C - Envia cadenas de caracteres al cliente, si envia la palabra exit cierra las comunicaciones
 * @version 0.1
 * @date 2022-10-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()

#define MAX 80
#define PORT 8080
#define SA struct sockaddr

// Funcion creada para comunicacion entre cliente y servidor
void func(int connfd)
{
	char buff[MAX];
	int n;
	
	// Bucle infinito para la comunicacion
	for (;;) {
		// Limpia el buffer
		bzero(buff, MAX);

		// Lee el mensaje del cliente y lo copia en el buffer
		read(connfd, buff, sizeof(buff));

		// Muestra el buffer el cual contiene el contenido del cliente
		printf("From client: %s\t To client : ", buff);
		bzero(buff, MAX);
		n = 0;

		// Copia el mensaje del servidor en el buffer
		while ((buff[n++] = getchar()) != '\n')
			;

		// Envia el contenido del buffer al cliente
		write(connfd, buff, sizeof(buff));

		// Si el mensaje contiene la palabra "Exit", el servidor finaliza la comunicacion y cierra el chat
		if (strncmp("exit", buff, 4) == 0) {
			printf("Server Exit...\n");
			break;
		}
	}
}

// Funcion principal del servidor
int main(int argc, char *argv[])
{
	int sockfd, connfd;
	socklen_t len;
	struct sockaddr_in servaddr, cli;

	// Creacion del socket y verificacion
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("Socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// Asignacion de direccion IP y puerto PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	// Permite el uso casi inmediato del socket y reutilizarlo
	const int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed");
		
	// Enlace del socket creado a la direccion IP y verificacion
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("Socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Ahora el servidor esta preparado para escuchar y verificacion
	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else {
		char str[40];
		printf("Server listening... %s:%d\n", inet_ntop(AF_INET, &servaddr.sin_addr.s_addr, str, sizeof(str)), htons(servaddr.sin_port));
	}

	

	// Se acepta la peticion del cliente y verificacion
	len = sizeof(cli);
	connfd = accept(sockfd, (SA*)&cli, &len);
	if (connfd < 0) {
		printf("Server accept failed...\n");
		exit(0);
	}
	else {
		char str[40];
		printf("Server accept the client... %s:%d\n", inet_ntop(AF_INET, &cli.sin_addr.s_addr, str, sizeof(str)), htons(cli.sin_port));
	}

	// Funcion creada para la comunicacion entre cliente y servidor
	func(connfd);

	// Una vez finalizada la comunicacion se cierra el socket
	close(sockfd);
}
