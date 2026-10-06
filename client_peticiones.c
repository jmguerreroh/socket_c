/**
 * @file client_peticiones.c
 * @author José Miguel Guerrero Hernández (josemiguel.guerrero@urjc.es)
 * @brief Cliente socket en C - Variante de client.c: UNA petición por conexión
 *
 * Diferencias con client.c (que mantiene una única conexión en bucle):
 *   - La IP y el puerto vienen como argumentos.
 *   - Cada petición abre su PROPIA conexión (socket + connect), envía una línea terminada en '\n',
 *     lee la respuesta hasta que el servidor cierra y cierra su socket.
 *   - enviar_texto() escribe TODO el texto y recibir_hasta_cierre() repite read() hasta el cierre.
 *
 * Uso:
 *     make server_peticiones client_peticiones
 *     ./server_peticiones 5000                   (en otra terminal)
 *     ./client_peticiones 127.0.0.1 5000
 *
 * El fichero se lee en dos partes: (1) funciones de envío y recepción y (2) programa principal, con la
 * función hacer_peticion() (paso C) que encapsula una petición completa.
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_PETICION   256         /* tamaño máximo de una petición (en bytes) */
#define MAX_RESPUESTA  32768       /* tamaño máximo de una respuesta (en bytes) */

/* =============== PARTE 1: envío y recepción por el socket =================================== */

/* El write() de func() de client.c, repetido hasta enviar TODO el texto. 0 = bien, -1 = error. */
int enviar_texto(int fd, const char *texto)
{
    size_t total = strlen(texto);
    size_t enviados = 0;

    while (enviados < total)
    {
        ssize_t n = write(fd, texto + enviados, total - enviados);
        if (n < 0)
        {
            if (errno == EINTR)     /* interrumpido por una señal: reintenta */
            {
                continue;
            }
            return -1;
        }
        enviados += (size_t)n;
    }
    return 0;
}

/* El read() de func() de client.c, repetido hasta que el servidor cierre. Devuelve los bytes o -1. */
int recibir_hasta_cierre(int fd, char *buffer, int tam)
{
    int recibidos = 0;

    while (recibidos < tam - 1)
    {
        ssize_t n = read(fd, buffer + recibidos, (size_t)(tam - 1 - recibidos));
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return -1;
        }
        if (n == 0)
        {
            break;                  /* el servidor ha cerrado: respuesta completa */
        }
        recibidos += (int)n;
    }
    buffer[recibidos] = '\0';
    return recibidos;
}

/* =============== PARTE 2: programa principal ================================================ */

/* PASO C: UNA petición = UNA conexión (el main de client.c: socket + connect, y luego func()).
 * Devuelve 0 si todo fue bien y -1 si falló; la respuesta queda en 'respuesta'. */
static int hacer_peticion(const char *ip, int puerto, const char *peticion,
                          char *respuesta, int tam)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_in servidor;
    memset(&servidor, 0, sizeof servidor);
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons((uint16_t)puerto);
    inet_pton(AF_INET, ip, &servidor.sin_addr);       /* en socket_c: inet_addr("127.0.0.1") */

    if (connect(fd, (struct sockaddr *)&servidor, sizeof servidor) < 0)
    {
        fprintf(stderr, "No se pudo contactar con el servidor en %s:%d (¿está en marcha?): %s\n",
                ip, puerto, strerror(errno));
        close(fd);
        return -1;
    }

    int r = -1;
    if (enviar_texto(fd, peticion) != 0)
    {
        fprintf(stderr, "Error al enviar la petición.\n");
    }
    else if (recibir_hasta_cierre(fd, respuesta, tam) < 0)
    {
        fprintf(stderr, "Error al recibir la respuesta.\n");
    }
    else
    {
        r = 0;
    }
    close(fd);                      /* cada petición cierra su socket */
    return r;
}

int main(int argc, char *argv[])
{
    /* ---- Argumentos (IP válida y puerto entre 1 y 65535) ---- */
    if (argc != 3)
    {
        fprintf(stderr, "Uso: %s IP PUERTO\n", argv[0]);
        return EXIT_FAILURE;
    }
    struct in_addr comprobar;
    int puerto = atoi(argv[2]);
    if (inet_pton(AF_INET, argv[1], &comprobar) != 1 || puerto < 1 || puerto > 65535)
    {
        fprintf(stderr, "IP o puerto no válidos.\n");
        return EXIT_FAILURE;
    }

    /* ---- Bucle: cada línea escrita es una petición, que se envía con hacer_peticion() ---- */
    static char respuesta[MAX_RESPUESTA];
    char texto[MAX_PETICION - 1];
    char peticion[MAX_PETICION];

    for (;;)
    {
        printf("Petición (SALIR para terminar): ");
        if (scanf(" %254[^\n]", texto) != 1)      /* lee una línea, con tamaño máximo */
        {
            break;                                 /* Ctrl+D */
        }
        snprintf(peticion, sizeof peticion, "%s\n", texto);   /* la petición acaba en '\n' */

        if (hacer_peticion(argv[1], puerto, peticion, respuesta, (int)sizeof respuesta) == 0)
        {
            printf("%s", respuesta);
        }
        if (strcmp(texto, "SALIR") == 0)
        {
            break;
        }
    }
    return EXIT_SUCCESS;
}
