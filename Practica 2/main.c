#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include "cJSON.h"
#include <time.h>

typedef struct{
    int no_cuenta;
    char nombre[25];
    float saldo;
}user;

pthread_mutex_t lock;
int a1 = 0;
int total_items_json = 0;
int total_items_json_tran=0;
int incremental=1;
int total_1=0; int total_2=0; int total_3=0;
int totalh1=0; int totalh2=0; int totalh3=0;
int totalht1=0; int totalht2=0; int totalht3=0;
int totalht4=0;
int buenas1=0; int buenas2=0; int buenas3=0;
int total_vacias=0; int buenas=0;
int actual_trans=0;
char filenamelog[100];
char filenamelog2[100];

// Function to read the entire contents of a file into a string
char *readFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Unable to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = (char *)malloc(length + 1);
    if (content == NULL) {
        perror("Unable to allocate memory");
        fclose(file);
        return NULL;
    }

    fread(content, 1, length, file);
    content[length] = '\0';

    fclose(file);
    return content;
}

void obtener_fecha_hora(char *buffer, size_t buffer_size) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strftime(buffer, buffer_size, "carga_%Y_%m_%d-%H_%M_%S.log", &tm);
}

void obtener_fecha_hora2(char *buffer, size_t buffer_size) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strftime(buffer, buffer_size, "operaciones_%Y_%m_%d-%H_%M_%S.log", &tm);
}

void obtener_fecha_hora_formato(char *buffer, size_t buffer_size) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strftime(buffer, buffer_size, "Fecha: %Y-%m-%d %H:%M:%S", &tm);
}

void limpiarBufferEntrada() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void leerArchivoBinario(int des, int cuenta) {
    FILE *archivo_binario = fopen("binario.bina", "rb");
    if (archivo_binario == NULL) {
        perror("Unable to open binary file");
        return;
    }

    user u;
    int cont=0;
    if(des==1){//busco cuenta
        while (fread(&u, sizeof(user), 1, archivo_binario)) {
            if (cont>=total_items_json)
            {
                break;
            }
            if(u.no_cuenta==cuenta){
                printf("-----------------------\n");
                printf("Cuenta: %d\n", u.no_cuenta);
                printf("Nombre: %s\n", u.nombre);
                printf("Saldo: %.2f\n", u.saldo);
                printf("-----------------------\n");
                break;
            }
            cont=cont+1;
        
        }
    }else{
        while (fread(&u, sizeof(user), 1, archivo_binario)) {
            if (cont>=total_items_json)
            {
                break;
            }
            if(u.no_cuenta==0){
                total_vacias = total_vacias + 1;
            }
            printf("-----------------------\n");
            printf("Cuenta: %d\n", u.no_cuenta);
            printf("Nombre: %s\n", u.nombre);
            printf("Saldo: %.2f\n", u.saldo);
            printf("-----------------------\n");
            
            cont=cont+1;
        
        }
    }
    

    fclose(archivo_binario);
}

int erroresTansaccion(int err, int item){
    FILE *file = fopen(filenamelog2, "r+");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    // Mover el cursor al final del archivo
    fseek(file, -1, SEEK_END);

    // Leer la última línea (opcional)
    char buffer[1024];
    fgets(buffer, sizeof(buffer), file);
    printf("Última línea: %s", buffer);

    // Escribir en la última línea
    if(err==1){
        fprintf(file, "Registro %d: Saldo insuficiente\n", item);

    }else if(err=2){
        fprintf(file, "Registro %d: Cuenta no existe\n", item);
    }else{
        fprintf(file, "Registro %d: Operacion no valida\n", item);
    }

    fclose(file);
    return 1;
}

void modificarBinario(char opc, int retiro, int deposito, float saldo) {
    FILE *archivo_binario = fopen("binario.bina", "rb+");
    if (archivo_binario == NULL) {
        perror("Unable to open binary file");
        return;
    }

    // 1 es DEPOSITO, 2 es RETIRO
    // 3 es TRANFERENCIA, 4 es INFORME
    user u;
    int cuenta1=0;int cuenta2=0;
    int saldo_cuenta1=0; int saldo_cuenta2=0;
    int cont = 0;
    while (fread(&u, sizeof(user), 1, archivo_binario)) {
        if (cont >= total_items_json) {
            break;
        }

        if (u.no_cuenta == retiro) {
            if (opc == '1') { // DEPOSITO
                printf("Saldo %.2f + %.2f\n", u.saldo, saldo);
                u.saldo = u.saldo + saldo;
                printf("Nuevo saldo %.2f\n", u.saldo);
                fseek(archivo_binario, -sizeof(user), SEEK_CUR);
                fwrite(&u, sizeof(user), 1, archivo_binario);
                break;
            } else if (opc == '2') { // RETIRO
                if (u.saldo >= saldo) {
                    u.saldo = u.saldo - saldo;
                    fseek(archivo_binario, -sizeof(user), SEEK_CUR); 
                    fwrite(&u, sizeof(user), 1, archivo_binario); 
                    break;
                } else {
                    erroresTansaccion(1,actual_trans);
                }
            }
        }
        if(opc >= '4'){
            erroresTansaccion(3,actual_trans);
        }
        if(opc == '3'){
            if (u.no_cuenta == retiro){
                cuenta1=1;
                if (u.saldo >= saldo)
                {
                    saldo_cuenta1=1;
                }else{
                    // operaciones no valida saldo insuficiente
                    erroresTansaccion(3,actual_trans);
                }
            }
            if (u.no_cuenta == deposito){
                cuenta2=1;
            }
        }
        if (cuenta1==1 && cuenta2==1 && saldo_cuenta1==1)
        {
            break;
        }
        cont = cont + 1;
    }
    if (cont >= total_items_json) {
        erroresTansaccion(2,actual_trans);
        printf("No se encontró la cuenta.\n");
    }
    fclose(archivo_binario);

    if(cuenta1==1 && cuenta2==1 && saldo_cuenta1==1){
        FILE *archivo_binario = fopen("binario.bina", "rb+");
        cont = 0;
        while (fread(&u, sizeof(user), 1, archivo_binario)) {
            if (cont >= total_items_json) {
                break;
            }

            if (u.no_cuenta == retiro) {// retirar
                u.saldo = u.saldo - saldo;
                fseek(archivo_binario, -sizeof(user), SEEK_CUR);
                fwrite(&u, sizeof(user), 1, archivo_binario);
            }
            if (u.no_cuenta == deposito) {// depositar
                u.saldo = u.saldo + saldo;
                fseek(archivo_binario, -sizeof(user), SEEK_CUR);
                fwrite(&u, sizeof(user), 1, archivo_binario);
            }
            cont = cont + 1;
        }

        fclose(archivo_binario);
    }
}

void cantidad_json(const char *filename){
    char *valor_json = readFile(filename);
    
    cJSON *json = cJSON_Parse(valor_json);
    free(valor_json);

    // Get the number of items in the JSON array
    total_items_json = cJSON_GetArraySize(json);
    printf("Total items in JSON array: %d\n", total_items_json);

    cJSON_Delete(json);
}

void cantidad_tran(){
    char *valor_json = readFile("transacciones.json");
    
    cJSON *json = cJSON_Parse(valor_json);
    free(valor_json);

    // Get the number of items in the JSON array
    total_items_json_tran = cJSON_GetArraySize(json);

    printf("Total items in JSON array: %d\n", total_items_json_tran);

    cJSON_Delete(json);
    srand(time(NULL));
    
    int lower = total_items_json_tran/6;
    int upper = total_items_json_tran/5;
    int hilo1 = (rand() % (upper - lower + 1)) + lower;
    
    lower = total_items_json_tran/4;
    upper = total_items_json_tran/3;
    int hilo2 = (rand() % (upper - lower + 1)) + lower;

    lower = total_items_json_tran/10;
    upper = total_items_json_tran/8;
    int hilo3 = (rand() % (upper - lower + 1)) + lower;


    int hilo4 = (total_items_json_tran - (hilo1 + hilo2 + hilo3));

    lower = 1;
    upper = 2;
    int des = (rand() % (upper - lower + 1)) + lower;
    if(des==1){
        totalht1=hilo4;
        totalht2=hilo2;
        totalht3=hilo3;
        totalht4=hilo1;
    }else{
        totalht1=hilo1;
        totalht2=hilo3;
        totalht3=hilo2;
        totalht4=hilo4;
    }
    printf("total:%d \n",total_items_json_tran);
    printf("hilo1:%d  hilo2:%d hilo3:%d  hilo4:%d\n",totalht1,totalht2,totalht3,totalht4);
}

int existe(const char *filename, const int cuenta){

    FILE *archivo_binario = fopen(filename, "rb");
    if (archivo_binario == NULL) {
        perror("Unable to open binary file");
        return 0;
    }

    user u;
    int cont=0;
    while (fread(&u, sizeof(user), 1, archivo_binario)) {
        if (cont>=total_items_json)
        {
            break;
        }
        
        if(u.no_cuenta==cuenta){
            printf("La cuenta %d ya existe\n", cuenta);
            fclose(archivo_binario);
            return 0;
        }
        cont=cont+1;
    }

    fclose(archivo_binario);
    
    return 1;
}

int salida_user(){
    
    obtener_fecha_hora(filenamelog, sizeof(filenamelog));
    
    FILE *file = fopen(filenamelog, "a+");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }
    

    char datetime[50];
    obtener_fecha_hora_formato(datetime, sizeof(datetime));

    // Escribe las líneas en el archivo
    fprintf(file, "------------- Carga de Usuarios -------------\n");
    fprintf(file, "%s\n", datetime);
    fprintf(file, "Usuarios cargados:\n");

    // Añadir líneas en blanco hasta la octava línea
    for (int i = 0; i < 1; i++) {
        fprintf(file, "\n");
    }

    fprintf(file, "Errores:\n");

    // Cierra el archivo
    fclose(file);
    return 0;
}


void crear_binario(){
    // Preparacion Bloque
    char bloque[1024];
    for (int i = 0; i < 1024; i++)
    {
        bloque[i] = '\0';
    }

    int limite = 0;
    FILE *archivo_binario;
    
    /*if(fopen(ubicacion_char,"r")!=NULL){
        cout<<"El archivo binario ya existe"<<endl;
        return;
    }*/

    cantidad_json("MOCK_DATA.json");
    srand(time(NULL));
    // Genera un número aleatorio entre 50 y 125
    int lower = total_items_json/4;
    int upper = total_items_json/2;
    int hilo1 = (rand() % (upper - lower + 1)) + lower;
    
    lower = total_items_json/8;
    upper = total_items_json/5;
    int hilo2 = (rand() % (upper - lower + 1)) + lower;

    int hilo3 = (total_items_json - (hilo1 + hilo2));

    lower = 1;
    upper = 2;
    int des = (rand() % (upper - lower + 1)) + lower;
    if(des==1){
        totalh1=hilo1;
        totalh2=hilo2;
        totalh3=hilo3;
    }else{
        totalh1=hilo1;
        totalh2=hilo3;
        totalh3=hilo2;
    }


    // Creo el archivo y escribo los bloques, peso = 10kb
    archivo_binario = fopen("binario.bina", "w");
    while (limite != 10)
    {
        fwrite(&bloque, 1024, 1, archivo_binario);
        limite++;
    }
    fclose(archivo_binario);
    salida_user();
}

int cargar_total(){
    
    FILE *file = fopen(filenamelog, "r+");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    char buffer[1024][256];  // Asume que el archivo tiene como máximo 1024 líneas y cada línea tiene como máximo 256 caracteres
    int num_lines = 0;

    // Leer todo el archivo en memoria
    while (fgets(buffer[num_lines], sizeof(buffer[num_lines]), file) != NULL) {
        num_lines++;
    }

    // Modificar la línea 4 (índice 3)
    int suma = total_1 + total_2 + total_3;
    if (num_lines >= 4) {
        snprintf(buffer[3], sizeof(buffer[3]), "Hilo 1: %d \nHilo 2: %d \nHilo 3: %d \nTotal: %d\n", total_1, total_2, total_3, suma);
        buffer[3][sizeof(buffer[3]) - 1] = '\0'; // Asegúrate de terminar correctamente la cadena
        strcat(buffer[3], "\n"); // Concatena un salto de línea al final si es necesario
    }

    // Mover el cursor al inicio del archivo
    fseek(file, 0, SEEK_SET);

    // Escribir todo el contenido de nuevo en el archivo
    for (int i = 0; i < num_lines; i++) {
        fputs(buffer[i], file);
    }

    // Truncar el archivo para eliminar datos sobrantes
    ftruncate(fileno(file), ftell(file));

    fclose(file);
    return 0;
}

int escribir_errores(int err, int item){
    FILE *file = fopen(filenamelog, "r+");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    // Mover el cursor al final del archivo
    fseek(file, -1, SEEK_END);

    // Leer la última línea (opcional)
    char buffer[1024];
    fgets(buffer, sizeof(buffer), file);
    printf("Última línea: %s", buffer);

    // Escribir en la última línea
    if(err==1){
        fprintf(file, "Registro %d: Numero de cuenta duplicado\n", item);

    }else{
        fprintf(file, "Registro %d: Saldo menor a 0\n", item);
    }

    fclose(file);
    return 1;
}

void* leer2(void* args){

    //printf("---Hilo: %s---\n", (char*) args);
    
    char *valor_json = readFile("MOCK_DATA.json");
    
    cJSON *json = cJSON_Parse(valor_json);
    free(valor_json);

    
    int num_item_actual=1;

    FILE *archivo_binario;
    user u;
    int num_time = 0;
    int actual = 1;

    // Iterate over each item in the array
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, json) {

        if(args=="1"){
            if(actual>totalh1){
                break;
            }
            actual=actual+1;
            buenas = buenas +1;
        }else if(args=="2"){
            if(actual<=totalh1 || actual>(totalh1+totalh2)){
                actual = actual +1;
                continue;
            }
            actual=actual+1;
            buenas = buenas +1;
        }else if(args=="3"){
            if(actual<=(totalh1+totalh2)){
                actual = actual +1;
                continue;
            }
            actual=actual+1;
            buenas = buenas +1;
        }

        cJSON *count = cJSON_GetObjectItemCaseSensitive(item, "no_cuenta");
        cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "nombre");
        cJSON *balance = cJSON_GetObjectItemCaseSensitive(item, "saldo");
        
        if (cJSON_IsNumber(count) && cJSON_IsString(name) && cJSON_IsNumber(balance)) {
            
            if((existe("binario.bina", count->valueint))==1){
                u.no_cuenta=count->valueint;
                strncpy(u.nombre, name->valuestring, sizeof(u.nombre) - 1);
                u.nombre[sizeof(u.nombre) - 1] = '\0'; // Ensure null-termination
                u.saldo=balance->valuedouble;

                // 1 para duplicado, 2 para error saldo
                if(u.saldo<0){
                    escribir_errores(2,actual);
                    continue;
                }

                // abro archivo y pego en mi binario
                archivo_binario = fopen("binario.bina","rb+");
                fseek(archivo_binario,0+(36*(actual-1)),SEEK_SET);
                fwrite(&u,sizeof(u),1,archivo_binario);
                fclose(archivo_binario);

                if(args=="1"){
                    total_1 = total_1 + 1;
                }
                if(args=="2"){
                    total_2 = total_2 + 1;
                }
                if(args=="3"){
                    total_3 = total_3 + 1;
                }
            }else{
                escribir_errores(1,actual);
            }

        } else {
            printf("Error al obtener datos del objeto JSON\n");
        }
        
    }
    
    cJSON_Delete(json);
}

void* leerTransacciones(void* args){
    
    pthread_mutex_lock(&lock);
    
    //printf("---Hilo: %s---\n", (char*) args);
    
    char *valor_json = readFile("transacciones.json");
    
    cJSON *json = cJSON_Parse(valor_json);
    free(valor_json);
    
    int num_item_actual=1;
    
    int num_time = 0;
    int actual = 1;

    if(args=="4"){
        // Reporte de Transacciones
        obtener_fecha_hora2(filenamelog2, sizeof(filenamelog2));
        
        FILE *file = fopen(filenamelog2, "a+");
        if (file == NULL) {
            perror("Error al abrir el archivo");
            //return 1;
        }
        

        char datetime[50];
        obtener_fecha_hora_formato(datetime, sizeof(datetime));

        // Escribe las líneas en el archivo
        fprintf(file, "------------- Carga de Operaciones -------------\n");
        fprintf(file, "Fecha: %s\n", datetime);
        fprintf(file, "Operaciones Por hilo:\n");
        fprintf(file, "Hilo 1: %d\n", totalht1);
        fprintf(file, "Hilo 2: %d\n", totalht2);
        fprintf(file, "Hilo 3: %d\n", totalht3);
        fprintf(file, "Hilo 4: %d\n", totalht4);
        fprintf(file, "Total: %d\n", totalht1+totalht2+totalht3+totalht4);

        fprintf(file, "Operaciones Realizadas:\n");
        fprintf(file, "Depositos: %d\n",buenas1);
        fprintf(file, "Retiros: %d\n",buenas2);
        fprintf(file, "Transferencias: %d\n",buenas3);
        fprintf(file, "Total: %d\n",buenas1+buenas2+buenas3);
        // Añadir líneas en blanco hasta la octava línea
        for (int i = 0; i < 1; i++) {
            fprintf(file, "\n");
        }
        fprintf(file, "------------------\n");
        fprintf(file, "Errores:\n");
        fclose(file);
    }

    // Iterate over each item in the array
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, json) {

        cJSON *operacion = cJSON_GetObjectItemCaseSensitive(item, "operacion");
        cJSON *cuenta1 = cJSON_GetObjectItemCaseSensitive(item, "cuenta1");
        cJSON *cuenta2 = cJSON_GetObjectItemCaseSensitive(item, "cuenta2");
        cJSON *monto = cJSON_GetObjectItemCaseSensitive(item, "monto");

        if(args=="1"){
            if(actual>totalht1){
                break;
            }
            actual=actual+1;
            buenas = buenas +1;
        }else if(args=="2"){
            if(actual<=totalht1 || actual>(totalht1+totalht2)){
                actual = actual +1;
                continue;
            }
            actual=actual+1;
            buenas = buenas +1;
        }else if(args=="3"){
            if(actual<=(totalht1+totalht2) || actual>(totalh1+totalh2+totalht3)){
                actual = actual +1;
                continue;
            }
            actual=actual+1;
            buenas = buenas +1;
        }else if(args=="4"){
            if(actual<=(totalh1+totalh2+totalht3)){
                actual = actual +1;
                continue;
            }
            actual=actual+1;
            buenas = buenas +1;
        }
        
        

        if (operacion->valueint==1)
        {
            buenas1 = buenas1 + 1;
        }
        if(monto->valuedouble<0){
            erroresTansaccion(1,actual);
        }
        
        
        
        if (cJSON_IsNumber(operacion) && cJSON_IsNumber(cuenta1) && cJSON_IsNumber(cuenta2)
            && cJSON_IsNumber(monto)) {
            /*printf("-------------: \n");
            printf("operacion: %d\n", operacion->valueint);
            printf("cuenta1: %d\n", cuenta1->valueint);
            printf("cuenta2: %d\n", cuenta2->valueint);
            printf("monto: %.2f\n", monto->valuedouble);*/

            if (operacion->valueint==1)
            {
                actual_trans = actual;
                modificarBinario('1', cuenta1->valueint, cuenta2->valueint, monto->valuedouble);
            }else if (operacion->valueint==2)
            {
                actual_trans = actual;
                modificarBinario('2', cuenta1->valueint, cuenta2->valueint, monto->valuedouble);
                buenas2 = buenas2 + 1;
            }else if(operacion->valueint==3){
                actual_trans = actual;
                modificarBinario('3', cuenta1->valueint, cuenta2->valueint, monto->valuedouble);
                buenas3 = buenas3 + 1;
            }else if(operacion->valueint>=5){
                erroresTansaccion(3,actual);
            }
        }
        
    }
    
    cJSON_Delete(json);
    
    pthread_mutex_unlock(&lock);
}

void cargaMasiva(){
    pthread_t  thread1, thread2, thread3;

    crear_binario();

    pthread_create(&thread1, NULL, leer2, "1");
    pthread_create(&thread2, NULL, leer2, "2");
    pthread_create(&thread3, NULL, leer2, "3");

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    //leer("MOCK_DATA.json");
    //leer2("MOCK_DATA.json");
    printf("Size of user struct: %lu bytes\n", sizeof(user));
    //leerArchivoBinario("binario.bina");

    printf("Total 1: %d \n", total_1);
    printf("Total 2: %d \n", total_2);
    printf("Total 3: %d \n", total_3);

    printf("Total items in JSON array: %d\n", total_items_json);
    
    printf("Buenas%d\n", buenas);

    
    cargar_total();
}

void operacionIndividual(){
    printf("######## Operacion individual #########\n");
    printf("# 1. Deposito                         #\n");
    printf("# 2. Retiro                           #\n");
    printf("# 3. Transaccion                      #\n");
    printf("# 4. Consultar Cuenta                 #\n");
    printf("#######################################\n");
    printf("Ingrese opción: ");

    char des;
    int des2, des3;
    float des4;
    while (1)
    {
        scanf("%c", &des);
        if (des=='1' || des=='2' || des=='3'||des=='4')
        {
            break;
        }
        
        
    }

    switch ( des ){
        case '1':
            printf("Ingrese el no. de cuenta a depositar: ");
            scanf("%d", &des2);
            printf("Ingrese el saldo a depositar: ");
            scanf("%f", &des4);
            modificarBinario('1',des2,0,des4);
            break;
        case '2':
            printf("Ingrese el no. de cuenta a retirar: ");
            scanf("%d", &des2);
            printf("Ingrese el saldo a retirar: ");
            scanf("%f", &des4);
            modificarBinario('2',des2,0,des4);
            break;
        case '3':
            printf("Ingrese el no. de cuenta a retirar: ");
            scanf("%d", &des2);
            printf("Ingrese el no. de cuenta a depositar: ");
            scanf("%d", &des3);
            printf("Ingrese el monto a transferir: ");
            scanf("%f", &des4);
            modificarBinario('3',des2,des3,des4);
            break;
        case '4':
            printf("Ingrese el no. de cuenta para informacion: ");
            scanf("%d", &des2);
            leerArchivoBinario(1,des2);
            break;
        default:
    }
    limpiarBufferEntrada();
    
}

void cargaMasivaOp(){

    cantidad_tran();
    pthread_mutex_init(&lock, NULL);
    pthread_t  thread1, thread2, thread3, thread4;

    pthread_create(&thread1, NULL, leerTransacciones, "1");
    pthread_create(&thread2, NULL, leerTransacciones, "2");
    pthread_create(&thread3, NULL, leerTransacciones, "3");
    pthread_create(&thread4, NULL, leerTransacciones, "4");

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    pthread_join(thread4, NULL);
    
    pthread_mutex_destroy(&lock);

    //
    FILE *archivo = fopen(filenamelog2, "r+");
    if (archivo == NULL) {
        perror("Unable to open file");
        exit(1);
    }

    // Crear un buffer para almacenar el contenido del archivo
    char buffer[1024];
    size_t length = fread(buffer, 1, sizeof(buffer), archivo);
    buffer[length] = '\0';
    rewind(archivo);

    // Buscar el final de la tercera línea para posicionarse en la cuarta
    int lineCount = 0;
    for (size_t i = 0; i < length; i++) {
        if (buffer[i] == '\n') {
            lineCount++;
        }
        if (lineCount == 9) {
            // Mover el puntero del archivo a la posición de la cuarta línea
            fseek(archivo, i + 1, SEEK_SET);
            break;
        }
    }

    // Escribir en la cuarta línea
    fprintf(archivo, "Depositos: %d\n",buenas1);
    fprintf(archivo, "Retiros: %d\n",buenas2);
    fprintf(archivo, "Transferencias: %d\n",buenas3);
    fprintf(archivo, "Total: %d\n",buenas1+buenas2+buenas3);

    fclose(archivo);
}

int reporte(){
    FILE *file = fopen("reporteUsuarios.json", "a+");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    // Escribe las líneas en el archivo
    fprintf(file, "[\n");

    FILE *archivo_binario = fopen("binario.bina", "rb");
    if (archivo_binario == NULL) {
        perror("Unable to open binary file");
        return 0;
    }

    user u;
    int cont = 0;
    while (fread(&u, sizeof(user), 1, archivo_binario)) {
        if (cont+1>total_items_json)
        {
            fprintf(file, "\t{\n");
            fprintf(file, "\t\t\"no_cuenta\": %d,\n", u.no_cuenta);
            fprintf(file, "\t\t\"nombre\":\"%s\",\n", u.nombre);
            fprintf(file, "\t\t\"saldo\": %.2f\n", u.saldo);
            fprintf(file, "\t}\n");
            break;
        }
        if(u.no_cuenta<=0){
            continue;
        }else{
            fprintf(file, "\t{\n");
            fprintf(file, "\t\t\"no_cuenta\": %d,\n", u.no_cuenta);
            fprintf(file, "\t\t\"nombre\": \"%s\",\n", u.nombre);
            fprintf(file, "\t\t\"saldo\": %.2f\n", u.saldo);
            fprintf(file, "\t},\n");
        }
        cont=cont+1;
    
    }

    fprintf(file, "\t{\n");
    fprintf(file, "\t\t\"no_cuenta\": %d,\n", u.no_cuenta);
    fprintf(file, "\t\t\"nombre\":\"%s\",\n", u.nombre);
    fprintf(file, "\t\t\"saldo\": %.2f\n", u.saldo);
    fprintf(file, "\t}\n");
    
    printf("total: %d, hasta:%d\n", total_items_json, cont);
    fprintf(file, "]\n");

    // Cierra el archivo
    fclose(file);

    printf("**************************************\n");
    printf("**************************************\n");
    printf("***** SE HA GENERADO EL REPORTE ******\n");
    printf("**************************************\n");
    printf("**************************************\n");
    return 0;
}

void menu(){
    while (1)
    {
    printf("################ MENU #################\n");
    printf("# 1. Carga Masiva de Usuarios         #\n");
    printf("# 2. Operacion Individual             #\n");
    printf("# 3. Carga Masiva de Operaciones      #\n");
    printf("# 4. Reporte                          #\n");
    printf("# 5. Salir                            #\n");
    printf("#######################################\n");
    printf("Ingrese opción: ");

    char des;
    scanf("%c", &des);
    if (des!='1' && des!='2' && des!='3'&& des!='4'&& des!='5')
    {
        continue;;
    }

    switch ( des ){
        case '1':
            cargaMasiva();
            break;
        case '2':
            operacionIndividual();
            break;
        case '3':
            cargaMasivaOp();
            break;
        case '4':
            reporte();
            break;
        case '5':
            // leerArchivoBinario(2,0);
            exit(0);
            break;
        default:
    }
    }
    limpiarBufferEntrada();
}

int main() {
    
    menu();

    return 0;
}
