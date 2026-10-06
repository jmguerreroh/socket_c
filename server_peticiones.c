/**
 * @file server_peticiones.c
 * @author José Miguel Guerrero Hernández (josemiguel.guerrero@urjc.es)
 * @brief Servidor socket en C - Variante de server.c: UNA petición por conexión
 *
 * Diferencias con server.c (que habla con un único cliente en bucle):
 *   - El puerto viene como argumento.
 *   - accept() va dentro de un bucle: por CADA conexión lee UNA línea (hasta '\n'), contesta y
 *     CIERRA. Si la línea es "SALIR", el servidor termina (también con Ctrl+C).
 *   - enviar_texto() escribe TODO el texto (un write() puede enviar menos de lo pedido).
 *   - recibir_linea() lee hasta '\n' con un tiempo máximo, en vez de suponer que un read() es un mensaje.
 *
 * Uso:
 *     make server_peticiones client_peticiones
 *     ./server_peticiones 5000                   (y en otra terminal: ./client_peticiones 127.0.0.1 5000)
 *
 * El fichero se lee en dos partes: (1) funciones de envío y recepción y (2) programa principal, con
 * los pasos numerados: crear el socket de escucha (A) y atender clientes en un bucle (B).
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_PETICION         256   /* tamaño máximo de una petición (en bytes) */
#define TIMEOUT_RECEPCION_S  3     /* segundos máximos esperando una petición */

/* =============== PARTE 1: envío y recepción por el socket =================================== */

/* El write() de func() de socket_c, repetido hasta enviar TODO el texto. 0 = bien, -1 = error. */
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

/* El read() de func() de socket_c, pero leyendo hasta el '\n'. Devuelve la longitud o -1. */
int recibir_linea(int fd, char *buffer, int tam)
{
    struct timeval limite = { TIMEOUT_RECEPCION_S, 0 };      /* tiempo máximo de espera */
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &limite, sizeof limite);

    int n = 0;
    for (;;)
    {
        char c;
        ssize_t leido = read(fd, &c, 1);
        if (leido < 0 && errno == EINTR)
        {
            continue;
        }
        if (leido <= 0)
        {
            return -1;              /* conexión cerrada, error o tiempo agotado */
        }
        if (c == '\n')
        {
            buffer[n] = '\0';
            return n;               /* longitud de la línea, sin el '\n' */
        }
        if (c == '\r')
        {
            continue;               /* ignora el retorno de carro */
        }
        if (n + 1 >= tam)
        {
            return -1;              /* la línea no cabe */
        }
        buffer[n++] = c;
    }
}

/* =============== PARTE 2: programa principal ================================================ */

/* Ctrl+C: pide terminar el bucle de atención de clientes */
static volatile sig_atomic_t g_parar = 0;

static void manejador_senal(int signo)
{
    (void)signo;
    g_parar = 1;
}

int main(int argc, char *argv[])
{
    /* ---- Argumentos (el puerto debe estar entre 1 y 65535) ---- */
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s PUERTO\n", argv[0]);
        return EXIT_FAILURE;
    }
    int puerto = atoi(argv[1]);
    if (puerto < 1 || puerto > 65535)
    {
        fprintf(stderr, "Puerto no válido: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    /* Ctrl+C pide cerrar (sin SA_RESTART para que accept() se interrumpa) y
     * escribir en un socket cerrado no mata el programa. */
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = manejador_senal;
    sigaction(SIGINT, &sa, NULL);
    signal(SIGPIPE, SIG_IGN);

    /* ---- PASO A: crear el socket de escucha (el principio de main de server.c) ---- */
    int fd_escucha = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_escucha < 0)
    {
        perror("socket");
        return EXIT_FAILURE;        /* aquí, libera lo que hayas reservado antes de salir */
    }

    int si = 1;
    setsockopt(fd_escucha, SOL_SOCKET, SO_REUSEADDR, &si, sizeof si);   /* reutilizar el puerto */

    struct sockaddr_in direccion;
    memset(&direccion, 0, sizeof direccion);
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = htonl(INADDR_ANY);
    direccion.sin_port = htons((uint16_t)puerto);

    if (bind(fd_escucha, (struct sockaddr *)&direccion, sizeof direccion) < 0 ||
        listen(fd_escucha, 5) < 0)
    {
        perror("bind/listen");
        close(fd_escucha);
        return EXIT_FAILURE;
    }
    printf("Servidor escuchando en el puerto %d (Ctrl+C para terminar)\n", puerto);

    /* ---- PASO B: aceptar clientes (el accept() de server.c, ahora dentro de un bucle) ---- */
    int salir = 0;
    while (!g_parar && !salir)
    {
        struct sockaddr_in dir_cliente;
        socklen_t largo = sizeof dir_cliente;
        int fd = accept(fd_escucha, (struct sockaddr *)&dir_cliente, &largo);
        if (fd < 0)
        {
            continue;               /* Ctrl+C u error pasajero: se vuelve a comprobar g_parar */
        }

        /* Quién se ha conectado, como "127.0.0.1:50422" (para el registro) */
        char ip[INET_ADDRSTRLEN];
        char cliente[INET_ADDRSTRLEN + 8];
        inet_ntop(AF_INET, &dir_cliente.sin_addr, ip, sizeof ip);
        snprintf(cliente, sizeof cliente, "%s:%d", ip, ntohs(dir_cliente.sin_port));

        char linea[MAX_PETICION];
        if (recibir_linea(fd, linea, (int)sizeof linea) < 0)
        {
            printf("[%s] Conexión descartada: no llegó una petición válida\n", cliente);
            close(fd);
            continue;
        }
        printf("[%s] Petición: %s\n", cliente, linea);

        /* >>> Aquí se atiende la petición: según 'linea' se decide qué hacer y se construye la
         *     respuesta. En este ejemplo solo se devuelve el mismo texto (eco) o se cierra con SALIR. <<< */
        char respuesta[MAX_PETICION + 64];
        if (strcmp(linea, "SALIR") == 0)
        {
            snprintf(respuesta, sizeof respuesta, "OK: servidor cerrado\n");
            salir = 1;
        }
        else
        {
            snprintf(respuesta, sizeof respuesta, "OK: he recibido '%s'\n", linea);
        }
        enviar_texto(fd, respuesta);

        close(fd);                  /* una petición = una conexión: cierra siempre */
    }

    close(fd_escucha);
    printf("Servidor finalizado.\n");
    return EXIT_SUCCESS;
}
