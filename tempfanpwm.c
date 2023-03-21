/*******************************************************************/
/* Nombre: tempfanpwm.c  (Temperature-Fan-pwm continuous control)  */
/* Descripcion: Control automatico de ventilador segun temperatura */
/*              con registro periodico del estado del programa y   */
/*              cambios de modos del ventilador mediante Log       */
/* Autor de la version original por tramos: JRios                  */
/* Fecha: 23/06/2017                                               */
/* Version: 1.0.2                                                  */
/* Autor de la presente versión con PWM continuo: Escrich          */
/* Fecha: 27/01/2023                                               */
/* Version: 2.0.0                                                  */
/* Descripcion: Control automatico de ventilador segun temperatura */
/*              con registro periodico del estado del programa     */
/*              cambios de velocidad del ventilador                */
/*              proporcional a la temperatura                      */
/*              Led que cambia de estado a cada ciclo (2 seg.)     */
/*              indicando que el programa está activo              */
/*              JMEC 20230127                                      */
/*******************************************************************/

// Librerias
#include <wiringPi.h>
#include <softPwm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <time.h>

// Definiciones (modificables según necesidades del usuario)
#define PIN_PWM 1                                         // Pin PWM numero 18, Wiring Pi pin nº 1
#define PWM_MIN 0                                         // Valor PWM minimo
#define PWM_MID 750                                       // Valor PWM intermedio [poner a 750 para Raspberry Pi]
#define PWM_MAX 1023                                      // Valor PWM maximo
#define PIN_LED 4                                         // Pin PWM numero 23 Wiring Pi pin nº 4 escrich
#define LED_ON 35                                         // Temperatura a partir de la cual enciendo el led verde escrich
#define LED_OFF 34                                        // Temperatura por debajo de la cual apago el led verde escrich
#define DIFF_TEMP 2                                       // Diferencia de temperatura minima entre lecturas (2ºC)
#define TEMP_LOW 20                                       // Valor umbral de temperatura bajo (40ºC) [Poner a 50 para Raspberry Pi] escrich
#define TEMP_HIGH 50                                      // Valor umbral de temperatura alto (55ºC) [Poner a 60 para Raspberry Pi] escrich
#define T_READS 5                                         // Tiempo de espera entre lecturas de temperatura (5s)
#define T_ALIVE 300                                        // 600// Tiempo de espera entre escritura en el archivo Log, para determinar que el programa esta en ejecucion (600s -> 10m)
#define FILE_TEMP "/sys/class/thermal/thermal_zone0/temp" // Ruta y nombre del archivo de acceso a la temperatura
#define FILE_LOG "/home/pi/ventilador.log"                // Ruta y nombre de archivo Log cambiado para verlo mas facilmente en &HOME escrich
#define LINE_SIZE 256                                     // Tamaño maximo de linea que puede ser escrita en el archivo Log
#define MAX_LINES 1000                                    // Numero maximo de lineas que puede contener el archivo Log (1000 lineas)

/*******************************************************************/

// Tipo de dato para determinar los estados correspondiente a la velocidad del ventilador (0%, 50% y 100%) tiende a desaparecer
typedef enum
{
    OFF,
    SLOW,
    FAST
} T_state;

// Prototipo de funciones
int readTemp(const char *file_path);                      // Funcion de lectura de la temperatura
void logPrintln(const char *file_path, char *data);       // Funcion para escribir una linea en el archivo Log
void logRemoveln(const char *file_path, int line_delete); // Funcion para eliminar una linea del archivo Log
int logLinesnum(const char *file_path);                   // Funcion para consultar el numero de lineas totales en el archivo log

/*******************************************************************/

// Funcion principal
int main(void)
{
    int temp_last, temp_now, diff_temp;       // Variables de temperatura
    T_state state_last, state_now;            // Variables de estado anterior y actual
    char log_writer[LINE_SIZE] = "";          // Cadena de caracteres para almacenar el mensaje de texto a escribir en el archivo Log
    time_t lastTime = time(NULL);             // Valor inicial de tiempo
    int valor_pwm, pwm_act, rango, estadoLed; // Valor a escribir en el pwm y auxiliares para calculos
    char led[1] = "x";                        // Variable para mostrar el estado del led en el log
    char encendido[1] = "1";                  // Variable para mostrar el estado del led en el log
    char apagado[1] = "0";                    // Variable para mostrar el estado del led en el log
    int porcentaje = 0;                       // Velociadad del ventilador expresada en porcentaje
    int minutos;                              // Tiempo entre lecturas dividido por 60 para obtener minutos

    logPrintln((char *)FILE_LOG, (char *)"Arrancando Escrich tempfanPWM...\n"); // Escribimos en el archivo Log

    wiringPiSetup();                          // Inicializamos la libreria WiringPi
    softPwmCreate(PIN_PWM, PWM_MIN, PWM_MAX); // Configuramos el pin "PIN_PWM" como salida PWM en el rango 0 - 1023
    pinMode(PIN_LED, OUTPUT);                 // Pin del led verde escrich

    logPrintln((char *)FILE_LOG, (char *)"GPIO configurado\n"); // Escribimos en el archivo Log

    softPwmWrite(PIN_PWM, PWM_MIN); // Ventilador apagado, aquí lo estamos poniendo a cero antes de empezar escrich
    state_now = OFF;                // Inicializamos el estado actual en OFF
    state_last = OFF;               // Inicializamos el ultimo estado en OFF
    digitalWrite(PIN_LED, HIGH);    // Encendemos inicialmente el led verde

    sprintf(log_writer, "Inicio completado, limites de temperatura:\n - Bajo  %d C\n - Alto  %d C\n", TEMP_LOW, TEMP_HIGH); // Preparamos lo que se va a escribir en el archivo Log
    logPrintln((char *)FILE_LOG, (char *)log_writer);                                                                       // Escribimos en el archivo Log

    temp_now = readTemp((char *)FILE_TEMP); // Leer temperatura

    minutos = T_ALIVE / 60; // Operacion para cambiar segundos a minutos

    while (1)
    {
        temp_now = readTemp((char *)FILE_TEMP); // Leer temperatura
        diff_temp = abs(temp_now - temp_last);  // Calcular diferencia de temperatura

        if (diff_temp >= DIFF_TEMP) // Diferencia de temperatura mayor que DIFF_TEMP (2ºC)
        {

            // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            //  Mi rutina de encendido y apagado de leds y control de la velocidad del ventilador
            // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

            rango = TEMP_HIGH - TEMP_LOW;
            valor_pwm = ((temp_now - TEMP_LOW) * PWM_MAX) / rango;
            if (valor_pwm > PWM_MAX)
                valor_pwm = PWM_MAX;
            if (valor_pwm < PWM_MIN)
                valor_pwm = PWM_MIN;

            porcentaje = (valor_pwm * 100) / PWM_MAX; // Si 1023 (PWM_MAX) es a 100, valor_pwm es a x

            // if (temp_now > LED_ON)
            //     estadoLed, LOW;
            // if (temp_now < LED_OFF)
            //     estadoLed, HIGH;

            if (estadoLed == LOW)
                estadoLed = HIGH;   // Enciende y apaga el led a cada ciclo para indicar que está funcionando
            else
                estadoLed = LOW;

            if (estadoLed == LOW)
                led[1] = encendido[1];
            else
                led[1] = apagado[1];

            digitalWrite(PIN_LED, estadoLed);

            // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

            softPwmWrite(PIN_PWM, valor_pwm); // Ventilador apagado

            sprintf(log_writer, "Temperatura actual  (%d C), ventilador (%% %d)\n", temp_now, porcentaje); // Preparamos lo que se va a escribir en el archivo Log
            logPrintln((char *)FILE_LOG, (char *)log_writer);                                              // Escribimos en el archivo Log
        }

        if ((time(NULL) - lastTime) >= T_ALIVE) // Si ha transcurrido T_ALIVE (1min) o más, escribimos en el Log un mensaje
        {
            sprintf(log_writer, "Estado del programa: Activo y funcionando desde hace %d minutos mas\n ", minutos); // Escribimos en el archivo Log
            logPrintln((char *)FILE_LOG, (char *)log_writer);

            sprintf(log_writer, "Temperatura actual = %d, Estado del pwm = %d, Estado del led = %d \n", temp_now, valor_pwm, led[0]); // Preparamos lo que se va a escribir en el archivo Log
            logPrintln((char *)FILE_LOG, (char *)log_writer);                                                                         // Escribimos en el archivo Log

            lastTime = time(NULL);
        }
        while (logLinesnum((char *)FILE_LOG) > MAX_LINES) // Mientras haya mas lineas en el archivo de Log que el maximo permitido
            logRemoveln((char *)FILE_LOG, 1);             // Se elimina la primera linea del archivo

        sleep(T_READS); // Esperamos, liberando a la CPU del proceso actual durante T_READS (5s)
    }

    return 0;
}

/*******************************************************************/

// Funcion de lectura de la temperatura
int readTemp(const char *file_path)
{
    FILE *file;   // Puntero a tipo archivo (para manejar el archivo)
    int temp = 0; // Variable donde almacenar la temperatura leida y para devolverla al salir de la funcion

    if ((file = fopen(file_path, "r")) != NULL) // Abrimos el archivo para leer
    {
        fscanf(file, "%d", &temp); // Almacenamos en la variable "temp" el primer valor entero (int) que haya en el archivo
        fclose(file);              // Cerramos el archivo
    }

    // Si la temperatura tiene 4 o mas digitos, dividimos entre 1000 (por ejemplo, en OPi Zero y Raspberry, se leen esos valores)
    if (temp >= 1000)
        temp = temp / 1000;

    return temp; // Devolvemos el valor de temperatura leida
}

/*******************************************************************/

// Funcion para escribir una linea en el archivo Log
void logPrintln(const char *file_path, char *data)
{
    FILE *file;                     // Puntero a tipo archivo (para manejar el archivo)
    time_t t_date = time(NULL);     // Variable de tipo fecha
    struct tm *tm_date;             // Estructura de datos de fecha
    char date[32] = "";             // Cadena de caracteres para almacenar la fecha en el formato que nos interesa
    char data_line[LINE_SIZE] = ""; // Cadena de caracteres para almacenar la linea de texto completa a escribir (fecha + datos)

    // Obtenemos la fecha de sistema para adjuntarla a los datos a escribir en la linea
    time(&t_date);
    tm_date = localtime(&t_date);
    sprintf(date, "[%d/%d/%d-%d:%d:%d] ", tm_date->tm_mday, tm_date->tm_mon + 1, tm_date->tm_year + 1900, tm_date->tm_hour, tm_date->tm_min, tm_date->tm_sec);

    // Preparamos la linea a escribir
    strcat(data_line, date); // Añadimos a "data_line" el contnido de "date"
    strcat(data_line, data); // Añadimos a "data_line" el contenido de "data"

    if ((file = fopen(file_path, "a")) != NULL) // Abrimos el archivo para añadir datos
    {
        fputs(data_line, file); // Escribimos los datos en el archivo
        fclose(file);           // Cerramos el archivo
    }
}

// Funcion para eliminar una linea del archivo Log
void logRemoveln(const char *file_path, int line_delete)
{
    FILE *file, *file_temp;         // Punteros a tipo archivo (para manejar los archivos)
    char tempFile[32] = "";         // Ruta y nombre del archivo temporal
    char line_read[LINE_SIZE] = ""; // Array de caracteres para almacenar cada lectura de linea (tamaño maximo de linea, 128 caracteres)
    int num_line = 1;               // Variable para llevar la cuenta de las lineas leidas
    int open_tempFile = 0;          // Variable para determinar si se ha podido crear el archivo temporal

    strcat(tempFile, file_path); // Añade al primer array el contenido del segundo ("/tmp/" + nombre del archivo original)
    strcat(tempFile, ".tmp");    // Añade la extension .tmp al nombre del archivo temporal

    if ((file = fopen(file_path, "r")) != NULL) // Abrir el archivo en modo lectura
    {
        if ((file_temp = fopen(tempFile, "w")) != NULL) // Abrir un archivo temporal en modo escritura
        {
            open_tempFile = 1;                                 // Archivo temporal abierto correctamente
            while (fscanf(file, "%[^\n]\n", line_read) != EOF) // Leemos cada linea hasta el final de fichero
            {
                if (num_line != line_delete) // Si la linea de lectura no es la linea a eliminar
                {
                    strcat(line_read, "\n");     // Añadimos el caracter de fin de linea (que no es leido con fs
                    fputs(line_read, file_temp); // Escribimos dicha linea en el archivo de escritura
                }
                num_line = num_line + 1; // Incrementamos el numero de linea leida
            }

            fclose(file_temp); // Cerramos el archivo de escritura temporal
        }
        fclose(file); // Cerramos el archivo original de lectura
    }

    if (open_tempFile) // Si se consiguio crear el archivo temporal
    {
        remove(file_path);           // Borramos el archivo original leido
        rename(tempFile, file_path); // Renombramos el archivo temporal con el nombre original
    }
}

// Funcion para consultar el numero de lineas totales en el archivo log
int logLinesnum(const char *file_path)
{
    FILE *file;                // Puntero a tipo archivo (para manejar el archivo)
    int num_lines = 0;         // Variable para contar el numero de lineas y para devolverla al salir de la funcion
    char line[LINE_SIZE] = ""; // Array de caracteres para almacenar cada lectura de linea (tamaño maximo de linea, 128 caracteres)

    if ((file = fopen(file_path, "r")) != NULL) // Abrir el archivo en modo lectura
    {
        while (fscanf(file, "%[^\n]\n", line) != EOF) // Leemos cada linea hasta el final de fichero
            num_lines = num_lines + 1;                // Incrementamos el numero de lineas

        fclose(file); // Cerramos el archivo
    }

    return num_lines; // Devolvemos el numero de lineas contadas, que tiene el archivo
}
