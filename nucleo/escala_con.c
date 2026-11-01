/* Escala de Notas — versión de consola (modo texto) para MS-DOS, FreeDOS, OS/2 Warp y cualquier sistema con compilador de C.
 * Menú numerado; exporta CSV, texto, RTF (Word) y HTML. Desarrollada por Javiera Navarrete. Licencia GPL-3.0-or-later. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "escala.h"

static Escala E;
static Curso CU;
static char titulo[64] = "Escala de notas";

static void linea(char *b, int n) { if (!fgets(b, n, stdin)) { b[0] = 0; return; } b[strcspn(b, "\r\n")] = 0; }
static double pedir(const char *txt, double def) {
    char b[40], d[24]; double v;
    esc_fmt(def, def == (long)def ? 0 : 2, d);
    printf("%s [%s]: ", txt, d); linea(b, sizeof b);
    if (!b[0]) return def;
    v = esc_num(b);
    return v < -1e8 ? def : v;
}
static void ver_tabla(void) {
    double *p = (double *)malloc(sizeof(double) * ESC_MAX_FILAS), *n = (double *)malloc(sizeof(double) * ESC_MAX_FILAS);
    int k, i, dec = esc_decimales(&E);
    char a[24], b[24];
    if (!p || !n) { printf("Sin memoria\n"); return; }
    k = esc_filas(&E, p, n, ESC_MAX_FILAS);
    printf("\n Puntaje  Nota\n -------  ----\n");
    for (i = 0; i < k; i++) {
        esc_fmt(p[i], p[i] == (long)p[i] ? 0 : 2, a); esc_fmt(n[i], dec, b);
        printf(" %7s  %s%s\n", a, b, n[i] < E.napr ? "  *" : "");
        if ((i + 1) % 20 == 0 && i + 1 < k) { printf(" -- Enter para seguir --"); { char t[8]; linea(t, 8); } }
    }
    esc_fmt(esc_puntos_para(&E, E.napr), 2, a);
    printf("\n * = bajo %.1f  -  Puntos para el %.1f: %s de %.0f\n", E.napr, E.napr, a, E.pmax);
    free(p); free(n);
}
static void parametros(void) {
    double v;
    E.pmax = pedir("Puntaje maximo", E.pmax);
    E.exig = pedir("Exigencia %", E.exig * 100) / 100;
    E.nmin = pedir("Nota minima", E.nmin); E.napr = pedir("Nota de aprobacion", E.napr); E.nmax = pedir("Nota maxima", E.nmax);
    v = pedir("Intervalo (1, 0,5 o 0,25)", E.paso); E.paso = (v == 0.5 || v == 0.25) ? v : 1;
    v = pedir("Redondeo (0 aproximar, 1 truncar, 2 dos decimales)", E.modo); E.modo = v == 1 ? ESC_TRUNC1 : v == 2 ? ESC_APROX2 : ESC_APROX1;
    if (!esc_valida(&E)) { printf("Datos invalidos: se restauran los valores por defecto.\n"); esc_defecto(&E); }
}
static void curso(void) {
    static char buf[6000]; char l[100]; int n = 0, i; char a[16], b[16];
    printf("Escribe una linea por estudiante: Nombre  puntaje. Termina con una linea vacia.\n");
    for (;;) { linea(l, sizeof l); if (!l[0]) break; if (n + (int)strlen(l) + 2 < (int)sizeof buf) { strcpy(buf + n, l); n += (int)strlen(l); buf[n++] = '\n'; buf[n] = 0; } }
    if (!esc_curso_leer(&E, buf, &CU)) { printf("Sin datos validos.\n"); return; }
    for (i = 0; i < CU.total; i++) { esc_fmt(CU.a[i].p, 2, a); esc_fmt(CU.a[i].n, esc_decimales(&E), b); printf(" %-30s %7s  %s\n", CU.a[i].nombre, a, b); }
    esc_fmt(CU.promedio, 2, a); printf("\nPromedio %s  ·  %d de %d aprobados\n", a, CU.aprobados, CU.total);
}
static void guardar(void) {
    char r[80]; int f;
    printf("Formato: 1 CSV  2 Texto  3 Word (RTF)  4 Web (HTML): "); linea(r, sizeof r); f = atoi(r) - 1;
    if (f < 0 || f > 3) return;
    printf("Titulo [%s]: ", titulo); linea(r, sizeof r); if (r[0]) { strncpy(titulo, r, 63); titulo[63] = 0; }
    { static const char *ext[4] = {"escala.csv", "escala.txt", "escala.rtf", "escala.htm"};
      printf(esc_exportar(&E, CU.total ? &CU : NULL, ext[f], f, titulo, "") ? "Guardado en %s\n" : "No se pudo guardar %s\n", ext[f]); }
}

int main(void) {
    char t[40];
    esc_defecto(&E); memset(&CU, 0, sizeof CU);
    printf("ESCALA DE NOTAS - calculo de notas escolares en Chile\nDesarrollada por Javiera Navarrete\n");
    for (;;) {
        char a[16];
        printf("\n1 Parametros  2 Ver tabla  3 Que nota es un puntaje  4 Puntos para una nota\n5 Notas del curso  6 Exportar  0 Salir\n> ");
        linea(t, sizeof t);
        switch (t[0]) {
        case '1': parametros(); break;
        case '2': ver_tabla(); break;
        case '3': { double p = pedir("Puntaje", 0); if (p < 0 || p > E.pmax) printf("Fuera de rango\n"); else { esc_fmt(esc_nota(&E, p), esc_decimales(&E), a); printf("Nota %s\n", a); } } break;
        case '4': { double q = esc_puntos_para(&E, pedir("Nota", 4)); if (q < 0) printf("No se alcanza\n"); else { esc_fmt(q, 2, a); printf("Desde %s puntos\n", a); } } break;
        case '5': curso(); break;
        case '6': guardar(); break;
        case '0': return 0;
        }
    }
}
