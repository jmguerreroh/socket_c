# Socket C - Comunicación Cliente-Servidor

Un ejemplo completo de comunicación TCP entre cliente y servidor implementado en C usando sockets.

## 📋 Descripción

Este proyecto implementa una comunicación bidireccional entre un cliente y un servidor usando sockets TCP en C. El servidor puede aceptar conexiones de clientes y mantener una conversación interactiva hasta que cualquiera de los dos envíe el comando "exit".

## 🚀 Características

- **Comunicación TCP**: Protocolo confiable para intercambio de mensajes
- **Bidireccional**: Tanto cliente como servidor pueden enviar mensajes
- **Manejo de errores**: Validación completa de operaciones de red
- **Entrada segura**: Uso de `fgets()` para evitar buffer overflow
- **Detección de desconexiones**: Manejo apropiado de desconexiones inesperadas
- **Reutilización de puerto**: Configuración `SO_REUSEADDR` para desarrollo

## 📁 Estructura del Proyecto

```
socket_c/
├── client.c         # Código fuente del cliente
├── server.c         # Código fuente del servidor
├── client_peticiones.c  # Variante: una petición por conexión (cliente)
├── server_peticiones.c  # Variante: una petición por conexión (servidor)
├── Makefile         # Automatización de compilación
└── README.md        # Este archivo
```

## 🛠️ Compilación

### Usando Makefile (recomendado)
```bash
make                 # Compila ambos programas
make server          # Solo el servidor
make client          # Solo el cliente
make clean           # Limpia ejecutables
```

### Compilación manual
```bash
gcc -Wall -o servidor server.c
gcc -Wall -o cliente client.c
```

## 💻 Uso

### 1. Iniciar el servidor
```bash
./servidor
```

El servidor mostrará:
```
Socket successfully created..
Socket successfully binded..
Server listening... 0.0.0.0:8080
```

### 2. Conectar el cliente
En otra terminal:
```bash
./cliente
```

El cliente mostrará:
```
Socket successfully created..
Connected to the server...127.0.0.1:8080
        To Server :
```

### 3. Comunicación
- El cliente envía un mensaje y espera respuesta del servidor
- El servidor recibe el mensaje, lo muestra y permite responder
- La comunicación continúa hasta que cualquiera envíe "exit"

## 📝 Ejemplo de Sesión

**Terminal del Servidor:**
```
Socket successfully created..
Socket successfully binded..
Server listening... 0.0.0.0:8080
Server accept the client... 127.0.0.1:45678
From Client: Hola servidor
         To Client : Hola cliente, ¿cómo estás?
From Client: Muy bien, gracias
         To Client : exit
Server Exit...
```

**Terminal del Cliente:**
```
Socket successfully created..
Connected to the server...127.0.0.1:8080
        To Server : Hola servidor
From Server : Hola cliente, ¿cómo estás?
        To Server : Muy bien, gracias
From Server : exit
Server closed connection...
```

## 🔁 Variante: una petición por conexión

`server.c` y `client.c` mantienen **una** conversación en bucle con un único cliente. Muchos servidores
(como las API sencillas por TCP) funcionan distinto: **cada petición usa su propia conexión**. Para ello están
`server_peticiones.c` y `client_peticiones.c`, que se compilan con `make` y se prueban entre sí:

```bash
./server_peticiones 5000                 # terminal 1
./client_peticiones 127.0.0.1 5000       # terminal 2: escribe una línea, recibe la respuesta; SALIR cierra el servidor
```

Qué cambia respecto a los ejemplos básicos:

| Ejemplo básico | Variante por petición |
|---|---|
| Puerto e IP fijos en el código | Vienen como argumentos (`argv`) |
| El servidor atiende a **un** cliente | `accept()` en un **bucle**: una conexión = una petición, y el servidor la **cierra** |
| Un `read()` = «el mensaje» | El servidor lee **hasta el `\n`** (`recibir_linea`, con tiempo máximo) y el cliente **hasta que el servidor cierra** (`recibir_hasta_cierre`) |
| Un `write()` | `enviar_texto`: `write()` en bucle hasta enviar todo |
| Nada de IP del cliente | El servidor muestra la **IP y puerto** de cada cliente (`inet_ntop` / `ntohs`) |
| `"exit"` | `SALIR` cierra el servidor; **Ctrl+C** también (sin `SA_RESTART`, para que `accept()` se interrumpa) |

Los dos ficheros están organizados en dos partes (funciones de envío y recepción, y programa principal) con los
pasos numerados (A: socket de escucha, B: bucle de `accept`, C: «hacer una petición»), pensados para explicarlos
uno a uno y reutilizarlos en otros programas.

## 🔧 Configuración

### Parámetros principales (definidos en el código):
- **Puerto**: 8080 (constante `PORT`)
- **Buffer**: 80 bytes (constante `MAX`)
- **IP del servidor**: 127.0.0.1 (localhost)
- **Cola de conexiones**: 5 conexiones pendientes

### Modificar configuración:
Para cambiar el puerto, edita la constante en ambos archivos:
```c
#define PORT 8080  // Cambiar por el puerto deseado
```

## 🛡️ Características de Seguridad

- **Prevención de buffer overflow**: Uso de `fgets()` con límite de tamaño
- **Validación de entrada**: Verificación de valores de retorno de todas las operaciones
- **Terminación segura de cadenas**: Garantía de terminación nula
- **Manejo de errores**: Uso de `perror()` para diagnóstico detallado
- **Limpieza de recursos**: Cierre apropiado de sockets

## 🐛 Solución de Problemas

### Error "Address already in use"
```bash
# El puerto está en uso, esperar unos segundos o usar:
sudo netstat -tulpn | grep :8080  # Ver qué proceso usa el puerto
```

### Error de conexión
- Verificar que el servidor esté ejecutándose
- Comprobar que no haya firewall bloqueando el puerto
- Verificar la IP y puerto de conexión


## 📚 Conceptos Técnicos

### Funciones principales utilizadas:
- `socket()`: Crear endpoint de comunicación
- `bind()`: Asociar socket a dirección
- `listen()`: Escuchar conexiones entrantes
- `accept()`: Aceptar conexión entrante
- `connect()`: Conectar a servidor remoto
- `read()/write()`: Intercambio de datos
- `close()`: Cerrar socket

### Flujo de comunicación:
1. **Servidor**: socket() → bind() → listen() → accept() → read()/write() → close()
2. **Cliente**: socket() → connect() → write()/read() → close()

## 👥 Autor

- **José Miguel Guerrero Hernández** (josemiguel.guerrero@urjc.es)
- Universidad Rey Juan Carlos
- Asignatura: Programación de Sistemas de Navegación
