# "all" es el objetivo por defecto. Sus dependencias se comprueban cuando ejecute la orden "make" en el directorio actual. Puede haber más de una dependencia.
all : server client server_peticiones client_peticiones

#### Reglas secundarias para compilar cada una de las dependencias.
server : server.c
	gcc -ggdb -Wall server.c -o server

client : client.c
	gcc -ggdb -Wall client.c -o client

server_peticiones : server_peticiones.c
	gcc -ggdb -Wall server_peticiones.c -o server_peticiones

client_peticiones : client_peticiones.c
	gcc -ggdb -Wall client_peticiones.c -o client_peticiones

# "clean" es un objetivo falso. Sirve para borrar los ejecutables.
clean :
	rm -f server client server_peticiones client_peticiones