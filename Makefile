# "all" es el objetivo por defecto. Sus dependencias se comprueban cuando ejecute la orden "make" en el directorio actual. Puede haber más de una dependencia.
all : server client

#### Reglas secundarias para compilar cada una de las dependencias.
server : server.c
	gcc -ggdb -Wall server.c -o server

client : client.c
	gcc -ggdb -Wall client.c -o client

# "clean" es un objetivo falso. Sirve para borrar los ejecutables.
clean :
	rm server client