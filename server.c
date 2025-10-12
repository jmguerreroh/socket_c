/**
 * @file server.c
 * @author Jose Miguel Guerrero Hernandez (josemiguel.guerrero@urjc.es)
 * @brief Servidor socket en C - Envía cadenas de caracteres al cliente, si envia la palabra exit cierra las comunicaciones
 * @version 0.2
 * @date 2022-10-18
 * 
 * @copyright Copyright (c) 2025
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

/**
 * Función para manejar la comunicación bidireccional entre servidor y cliente
 * @param connfd Descriptor del socket de conexión con el cliente
 */
void func(int connfd)
{
	char buff[MAX];
	
	// Bucle principal de comunicación servidor-cliente
	for (;;) {
		// Inicializa el buffer a ceros para limpiar datos previos
		bzero(buff, MAX);

		// Lee el mensaje del cliente y verifica errores/desconexiones
		int bytes_read = read(connfd, buff, sizeof(buff) - 1);
		if (bytes_read <= 0) {
			if (bytes_read == 0) {
				printf("Client disconnected\n");
			} else {
				perror("Error reading data");
			}
			break;
		}
		buff[bytes_read] = '\0'; // Asegurar terminación nula de la cadena

		// Muestra el mensaje recibido del cliente y solicita respuesta
		printf("From Client: %s\t To Client : ", buff);
		
		// Limpia el buffer para prepararlo para la respuesta del servidor
		bzero(buff, MAX);

		// Lee la respuesta del servidor desde teclado de forma segura
		if (fgets(buff, sizeof(buff), stdin) == NULL) {
			printf("Error reading input\n");
			break;
		}

		// Envía la respuesta del servidor al cliente
		if (write(connfd, buff, strlen(buff)) < 0) {
			perror("Error sending data");
			break;
		}

		// Si el servidor envía "exit", finaliza la comunicación
		if (strncmp("exit", buff, 4) == 0) {
			printf("Server Exit...\n");
			break;
		}
	}
}

// Función principal del servidor
int main(int argc, char *argv[])
{
	int sockfd, connfd;
	socklen_t len;
	struct sockaddr_in servaddr, cli;

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

	// Configuración de la dirección del servidor (cualquier IP, puerto PORT)
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	// Permite reutilizar inmediatamente el puerto después del cierre
	const int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed");
		
	// Vincula el socket a la dirección y puerto especificados
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		perror("Socket bind failed");
		close(sockfd);
		exit(1);
	}
	else
		printf("Socket successfully binded...\n");

	// Configura el socket para escuchar conexiones entrantes (máximo 5 en cola)
	if ((listen(sockfd, 5)) != 0) {
		perror("Listen failed");
		close(sockfd);
		exit(1);
	}
	else {
		char str[40];
		printf("Server listening... %s:%d\n", inet_ntop(AF_INET, &servaddr.sin_addr.s_addr, str, sizeof(str)), htons(servaddr.sin_port));
	}

	

	// Acepta la conexión entrante del cliente
	len = sizeof(cli);
	connfd = accept(sockfd, (SA*)&cli, &len);
	if (connfd < 0) {
		perror("Server accept failed");
		close(sockfd);
		exit(1);
	}
	else {
		char str[40];
		printf("Server accept the client... %s:%d\n", inet_ntop(AF_INET, &cli.sin_addr.s_addr, str, sizeof(str)), htons(cli.sin_port));
	}

	// Inicia la comunicación bidireccional con el cliente
	func(connfd);

	// Limpieza: cierra ambos sockets al finalizar la comunicación
	close(connfd);
	close(sockfd);
}
