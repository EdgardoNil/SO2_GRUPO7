#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define FILENAME "practica1.txt"

// Función que genera carácter`s aleatorio
char random_char() {
    return 'A' + rand() % 26;
}

// Función para limpiar el  archivo
void limpiar_archivo(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }
    fclose(file);
}

// Función para realizar una escritura aleatoria en el archivo
void write_random_string(int fd, int child_pid) {
    char buffer[9];
    for (int i = 0; i < 8; i++) {
        buffer[i] = random_char();
    }
    buffer[8] = '\n';
    write(fd, buffer, 9);
}

// Función para realizar una lectura aleatoria desde el archivo
void read_string(int fd, int child_pid) {
    char buffer[9];
    ssize_t bytes_read = read(fd, buffer, 8);
    if (bytes_read == -1) {
        perror("Error al leer desde el archivo");
        exit(EXIT_FAILURE);
    }
    buffer[bytes_read] = '\0';
}

// Función para abrir el archivo
int abrir_archivo(const char *filename, int child_pid) {
    int fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }
    return fd;
}

// Función para realizar una operación aleatoria (open, lectura o escritura)
void perform_random_operation(int fd, int child_pid) {
    int operation = rand() % 3; // Generar un número aleatorio entre 0 y 2
    switch (operation) {
        case 0:
            abrir_archivo(FILENAME, child_pid);
            break;
        case 1:
            write_random_string(fd, child_pid);
            break;
        case 2:
            read_string(fd, child_pid);
            break;
    }
}

int main() {
   
    // srand(time(NULL) ^ getpid());

    //============================= Apertura del archivo =============================
    
    int fd = open(FILENAME, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

      // Limpiar el contenido del archivo antes de empezar
    limpiar_archivo(FILENAME);

    int child_pid = getpid();
    printf("Proceso hijo con PID: %d\n", child_pid);

    //============================= Bucle principal =============================
    
    while (1) { 
        perform_random_operation(fd, child_pid);
        sleep(1 + rand() % 3); // Esperar de 1 a 3 segundos antes de la próxima operación
    }

    close(fd);
    return 0;
}