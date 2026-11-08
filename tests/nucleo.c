/* Pruebas del núcleo C: lectura del curso, avisos y exportación segura. Salida distinta de 0 si algo falla. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../nucleo/escala.h"

static int fallos = 0;
#define VERIFICAR(cond, msg) do { if (!(cond)) { printf("FALLA: %s\n", msg); fallos++; } } while (0)

static char *leer_archivo(const char *ruta) {
    static char buf[16384]; size_t n; FILE *f = fopen(ruta, "rb");
    if (!f) return NULL;
    n = fread(buf, 1, sizeof buf - 1, f); buf[n] = 0; fclose(f); return buf;
}

int main(void) {
    static Escala e; static Curso c;
    const char *texto = "Ana Perez  48\nBenjamin Soto 31,5\nSolo nombre\nCarla 480\n\n=HYPERLINK(\"http://x\")  50\n";
    char *s;
    esc_defecto(&e);

    VERIFICAR(esc_curso_leer(&e, texto, &c) == 4, "debe leer 4 estudiantes");
    VERIFICAR(c.ignoradas == 1, "la línea sin puntaje debe contarse como ignorada");
    VERIFICAR(c.ajustados == 1, "480 con máximo 60 debe contarse como ajustado");
    VERIFICAR(c.a[2].p == 60, "480 se lleva al máximo");
    VERIFICAR(esc_nota(&e, 36) == 4.0, "36 de 60 con 60 % es un 4,0");

    /* la fórmula del nombre no debe salir como fórmula en el CSV */
    VERIFICAR(esc_exportar(&e, &c, "prueba-nucleo.csv", ESC_CSV, "=Titulo", ""), "exportar CSV");
    s = leer_archivo("prueba-nucleo.csv");
    VERIFICAR(s != NULL, "leer el CSV");
    if (s) {
        VERIFICAR(strstr(s, ";'=Titulo") == NULL, "el título se escribe tal cual");
        VERIFICAR(strstr(s, "\"'=Titulo\"") != NULL, "título que empieza con = lleva apóstrofo y comillas");
        VERIFICAR(strstr(s, "\"'=HYPERLINK(\"\"http://x\"\")\";") != NULL, "nombre con fórmula neutralizado y comillas duplicadas");
        VERIFICAR(strstr(s, "Ana Perez;48;") != NULL, "nombre normal sin cambios");
    }
    remove("prueba-nucleo.csv");

    /* errores de escritura se informan */
    VERIFICAR(!esc_exportar(&e, &c, "/no/existe/x.csv", ESC_CSV, "t", ""), "ruta inválida devuelve 0");
    VERIFICAR(!esc_exportar(&e, &c, "/dev/full", ESC_TXT, "t", ""), "disco lleno devuelve 0");

    if (fallos) { printf("%d fallas\n", fallos); return 1; }
    printf("nucleo: pruebas OK\n");
    return 0;
}
