/**
 * @file client.c
 * @author José Miguel Guerrero Hernández (josemiguel.guerrero@urjc.es)
 * @brief Cliente socket en C - Envía cadenas de caracteres al servidor
 * @version 0.2
 * @date 2022-10-18
 * 
 * @copyright Copyright (c) 2025
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

/**
 * Función para manejar la comunicación bidireccional entre cliente y servidor
 * @param sockfd Descriptor del socket conectado al servidor
 */
void func(int sockfd)
{
	char buff[MAX];
	
	// Bucle principal de comunicación cliente-servidor
	for (;;) {
		// Inicializa el buffer a ceros para limpiar datos previos
		bzero(buff, sizeof(buff));
		printf("\tTo Server : ");

		// Copia el mensaje del cliente en el buffer de forma segura
		if (fgets(buff, sizeof(buff), stdin) == NULL) {
			printf("Error reading input\n");
			break;
		}

		// Envia el contenido del buffer al servidor
		if (write(sockfd, buff, strlen(buff)) < 0) {
			perror("Error sending data");
			break;
		}
		
		// Limpia el buffer para prepararlo para la respuesta del servidor
		bzero(buff, sizeof(buff));

		// Lee el mensaje del servidor y lo copia en el buffer
		int bytes_read = read(sockfd, buff, sizeof(buff) - 1);
		if (bytes_read <= 0) {
			if (bytes_read == 0) {
				printf("Server disconnected\n");
			} else {
				perror("Error reading data");
			}
			break;
		}
		buff[bytes_read] = '\0'; // Asegurar terminación nula de la cadena
		printf("From Server : %s", buff);

		// Si el mensaje del servidor contiene "exit", finaliza la comunicación
		if (strncmp("exit", buff, 4) == 0) {
			printf("Client Exit...\n");
			break;
		}
	}
}

// Función principal del cliente
int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in servaddr;

	// Creación del socket TCP y verificación
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("Socket creation failed");
		exit(1);
	}
	else
		printf("Socket successfully created..\n");
	
	// Inicializar la estructura de dirección del servidor
	bzero(&servaddr, sizeof(servaddr));

	// Asignación de dirección IP y puerto PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	// Conexión entre el socket del cliente y el socket del servidor
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		perror("Connection with the server failed");
		close(sockfd);
		exit(1);
	}
	else {
		char str[40];
		printf("Connected to the server...%s:%d\n", inet_ntop(AF_INET, &servaddr.sin_addr.s_addr, str, sizeof(str)), htons(servaddr.sin_port));
	}

	// Función creada para la comunicación entre cliente y servidor
	func(sockfd);

	// Cierre del socket
	close(sockfd);
}
