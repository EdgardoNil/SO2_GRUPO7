#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define MAX_LINE_LENGTH 100

int total_syscalls = 0;
int read_syscalls = 0;
int write_syscalls = 0;
int open_syscalls = 0;

// Manejador de la señal SIGINT para imprimir las estadísticas
void sigint_handler(int signum) {
    printf("\nNúmero total de llamadas al sistema realizadas por los procesos hijo: %d\n", total_syscalls);
    printf("Número de llamadas al sistema por tipo:\n");
    printf("Read: %d\n", read_syscalls);
    printf("Write: %d\n", write_syscalls);
    printf("open: %d\n", open_syscalls);
    exit(EXIT_SUCCESS);
}

// Función para actualizar los contadores de llamadas al sistema según la línea leída
void actualizar_contadores(const char *line) {
    if (strstr(line, "read") != NULL) {
        read_syscalls++;
    } else if (strstr(line, "write") != NULL) {
        write_syscalls++;
    } else if (strstr(line, "open") != NULL) {
        open_syscalls++;
    }
    total_syscalls++;
}

// Función para limpiar el archivo syscalls.log y, si no existe, crearlo
void limpiar_archivo_syscalls_log() {
    FILE *file = fopen("syscalls.log", "w");
    if (file == NULL) {
        perror("Error al abrir el archivo syscalls.log");
        exit(EXIT_FAILURE);
    }
    fclose(file);
}

// Función para asegurar que el archivo syscalls.log existe
void asegurar_syscalls_log() {
    FILE *file = fopen("syscalls.log", "r");
    if (file == NULL) {
        limpiar_archivo_syscalls_log(); // Si el archivo no existe, lo crea
    } else {
        fclose(file);
    }
}

int main() {
    // Registrar el manejador de señales SIGINT
    signal(SIGINT, sigint_handler);

    //printf("Antes de crear los procesos hijos\n");

    // Crear el primer proceso hijo
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid1 == 0) {
        // Proceso hijo 1
        char *arg_Ptr[3];
        arg_Ptr[0] = "./hijo.bin";
        arg_Ptr[1] = "1";
        arg_Ptr[2] = NULL;
        execv(arg_Ptr[0], arg_Ptr);
        perror("execv");
        exit(EXIT_FAILURE);
    }

    // Crear el segundo proceso hijo
    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid2 == 0) {
        // Proceso hijo 2
        char *arg_Ptr[3];
        arg_Ptr[0] = "./hijo.bin";
        arg_Ptr[1] = "2";
        arg_Ptr[2] = NULL;
        execv(arg_Ptr[0], arg_Ptr);
        perror("execv");
        exit(EXIT_FAILURE);
    }

    printf("Soy el proceso padre con PID: %d\n", getpid());

    // Limpiar el archivo syscalls.log si ya existe
    limpiar_archivo_syscalls_log();

    // Ejecución del script SystemTap para monitorear llamadas al sistema
    char command[100];
    sprintf(command, "%s %d %d %s", "sudo stap tap.stp ", pid1, pid2, " > syscalls.log");
    system(command);

    // Leer el archivo syscalls.log y actualizar los contadores
    FILE *file = fopen("syscalls.log", "r");
    if (file == NULL) {
        perror("Error al abrir el archivo syscalls.log");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        actualizar_contadores(line);
    }
    fclose(file);

    printf("\n");
    printf("Esperando a que los procesos hijos terminen\n");

    // Esperar a que los procesos hijos terminen
    while (1) {
        int status;
        pid_t pid = wait(&status);
        if (pid == -1) {
            if (errno == ECHILD) {
                // No hay más hijos
                break;
            } else {
                perror("wait");
                exit(EXIT_FAILURE);
            }
        }
        
        if (WIFEXITED(status)) {
            // Incrementar el número total de llamadas al sistema
            total_syscalls += WEXITSTATUS(status);
            // Contar las llamadas al sistema por tipo
            if (WEXITSTATUS(status) == 1) {
                read_syscalls++;
            } else if (WEXITSTATUS(status) == 2) {
                write_syscalls++;
            }
        } else {
            printf("El proceso hijo con PID %d terminó de manera anormal\n", pid);
        }
    }

    // Al recibir SIGINT, el manejador de señales imprimirá las estadísticas
    return 0;
}