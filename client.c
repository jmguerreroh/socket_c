/**
 * @file client.c
 * @author Jose Miguel Guerrero Hernandez (josemiguel.guerrero@urjc.es)
 * @brief Cliente socket en C - Envia cadenas de caracteres al servidor
 * @version 0.1
 * @date 2022-10-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#define MAX 80
#define PORT 8080
#define SA struct sockaddr

// Funcion creada para comunicacion entre cliente y servidor
void func(int sockfd)
{
	char buff[MAX];
	int n;
	for (;;) {
		// Limpia el buffer
		bzero(buff, sizeof(buff));
		printf("\tTo Server : ");
		n = 0;

		// Copia el mensaje del cliente en el buffer		
		while ((buff[n++] = getchar()) != '\n')
			;

		// Envia el contenido del buffer al servidor
		write(sockfd, buff, sizeof(buff));
		bzero(buff, sizeof(buff));

		// Lee el mensaje del servidor y lo copia en el buffer
		read(sockfd, buff, sizeof(buff));
		printf("From Server : %s", buff);

		// Si el mensaje contiene la palabra "Exit", el servidor finaliza y cierra el chat
		if (strncmp("exit", buff, 4) == 0) {
			printf("Client Exit...\n");
			break;
		}
	}
}

// Funcion principal del cliente
int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in servaddr;

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
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	// Conexion entre el socket del cliente y el socket del servidor
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("Connection with the server failed...\n");
		exit(0);
	}
	else {
		char str[40];
		printf("Connected to the server...%s:%d\n", inet_ntop(AF_INET, &servaddr.sin_addr.s_addr, str, sizeof(str)), htons(servaddr.sin_port));
	}

	// Funcion creada para la comunicacion entre cliente y servidor
	func(sockfd);

	// Cierra del socket
	close(sockfd);
}
