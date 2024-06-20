#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER_SIZE 1024

typedef struct {
    int pid;
    char proceso[256];
    char llamada[256];
    float tamano;     
    char fecha_hora[20];
    float porcentaje; 
} Solicitud;

void finish_with_error(MYSQL *conn) {
    fprintf(stderr, "%s\n", mysql_error(conn));
    mysql_close(conn);
    exit(1);
}

// Función para insertar la solicitud en la base de datos MySQL
void insertar_DBMS(MYSQL *conn, Solicitud solicitud) {
    char query[MAX_BUFFER_SIZE];
    sprintf(query, "INSERT INTO solicitud (pid, proceso, llamada, tamano, fecha_hora, porcentaje) VALUES (%d, '%s', '%s', %.2f, '%s', %.2f)",
        solicitud.pid, solicitud.proceso, solicitud.llamada, solicitud.tamano, solicitud.fecha_hora, solicitud.porcentaje);

    
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error al insertar datos: %s\n", mysql_error(conn));
        exit(1);
    }

    printf("Datos insertados correctamente en la base de datos\n");
}

int main() {
    MYSQL *conn;

    // Configuración de la conexión
    char *server = "127.0.0.1"; 
    char *user = "root";        
    char *password = "SO2P1$";
    char *database = "Memoria";

    // Inicializar conexión MySQL
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "Error al inicializar la conexión MySQL\n");
        exit(1);
    }

    // Conectar a la base de datos
    if (mysql_real_connect(conn, server, user, password, database, 3306, NULL, 0) == NULL) {
        finish_with_error(conn);
    }

    // Obtener la memoria total del sistema
    int memoriaTotalSitema;
    FILE *mem_fp = popen("free -m | awk 'NR==2{print $2}'", "r");
    if (mem_fp == NULL) {
        fprintf(stderr, "Error al abrir el comando 'free'\n");
        exit(1);
    }
    fscanf(mem_fp, "%d", &memoriaTotalSitema);
    pclose(mem_fp);

    // Convertir la memoria total del sistema de MB a KB
    int memoriaKB = memoriaTotalSitema * 1024;

    // Imprimir el valor de la memoria total del sistema
    printf("Memoria total del sistema: %d MB\n", memoriaKB);


    // Abrir el script de SystemTap
    FILE *fp;
    char buffer[MAX_BUFFER_SIZE];

    fp = popen("sudo ./meminfo.stp", "r");
    if (fp == NULL) {
        fprintf(stderr, "Error al abrir el script de SystemTap\n");
        exit(1);
    }

    // Leer la salida del script línea por línea
    while (fgets(buffer, MAX_BUFFER_SIZE, fp) != NULL) {
        // Procesar la línea para extraer la información necesaria y almacenarla en una estructura Solicitud
        Solicitud solicitud;
        if (sscanf(buffer, "PID del proceso: %d", &solicitud.pid) == 1) {
            // Si la línea contiene el PID del proceso
            // Leer la siguiente línea que contiene el nombre del proceso
            fgets(buffer, MAX_BUFFER_SIZE, fp);
            sscanf(buffer, "Nombre del proceso: %s", solicitud.proceso);

            // Leer la siguiente línea que contiene la llamada
            fgets(buffer, MAX_BUFFER_SIZE, fp);
            sscanf(buffer, "Llamada: %[^()]s", solicitud.llamada); // Esto capturará todo hasta encontrar los paréntesis

            // Leer la siguiente línea que contiene el tamaño del segmento de memoria solicitado
            fgets(buffer, MAX_BUFFER_SIZE, fp);
            sscanf(buffer, "Tamaño del segmento de memoria solicitado: %f", &solicitud.tamano);
            // solicitud.tamano /= 1024; // Convertir el tamaño a MB

            // Calcular el porcentaje
            solicitud.porcentaje = solicitud.tamano / memoriaKB * 100;

            // Leer la siguiente línea que contiene la fecha y hora de la solicitud
            fgets(buffer, MAX_BUFFER_SIZE, fp);
            char *inicio_fecha_hora = strstr(buffer, "Fecha y hora de la solicitud: ");
            if (inicio_fecha_hora != NULL) {
                // Mover el puntero al comienzo de la fecha y hora
                inicio_fecha_hora += strlen("Fecha y hora de la solicitud: ");
                // Copiar la fecha y hora en la estructura de Solicitud
                strncpy(solicitud.fecha_hora, inicio_fecha_hora, sizeof(solicitud.fecha_hora) - 1);
                solicitud.fecha_hora[sizeof(solicitud.fecha_hora) - 1] = '\0'; // Asegurar que la cadena termina con nulo
            } else {
                // Error: no se encontró el encabezado esperado en la línea
                fprintf(stderr, "Error: no se encontró el encabezado esperado en la línea\n");
            }

            // Insertar la solicitud en la base de datos MySQL
            insertar_DBMS(conn, solicitud);
        }
    }

    // Cerrar el flujo y la conexión a la base de datos
    pclose(fp);
    mysql_close(conn);

    return 0;
}
