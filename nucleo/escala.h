/* Escala de Notas — núcleo portable en C89 (DOS, Windows 3.x / 9x / 2000–8, OS/2). La misma fórmula que la interfaz
 * web: escala por tramos chilena (nota mínima → aprobación en el puntaje de exigencia → nota máxima).
 * Desarrollada por Javiera Navarrete. Licencia GPL-3.0-or-later. */
#ifndef ESCALA_H
#define ESCALA_H

#define ESC_APROX1 0     /* aproximar a un decimal (3,95 → 4,0) — lo habitual */
#define ESC_TRUNC1 1     /* truncar a un decimal */
#define ESC_APROX2 2     /* dos decimales */

typedef struct {
    double pmax;     /* puntaje máximo */
    double exig;     /* exigencia, 0..1 */
    double paso;     /* intervalo de puntaje: 1, 0,5 o 0,25 */
    double nmin, napr, nmax;
    int modo;
} Escala;

#ifdef __386__
#define ESC_MAX_FILAS 1601          /* 32 bits: hasta 400 puntos en cuartos de punto */
#else
#define ESC_MAX_FILAS 641           /* 16 bits: hasta 160 puntos en cuartos de punto */
#endif
#define ESC_MAX_CURSO 120

typedef struct { char nombre[40]; double p, n; } Alumno;
typedef struct {
    Alumno a[ESC_MAX_CURSO]; int total;
    double promedio, mediana, nota_baja, nota_alta; int aprobados; int bins[6];
} Curso;

void   esc_defecto(Escala *e);
int    esc_valida(const Escala *e);                      /* 1 si los datos tienen sentido */
double esc_nota(const Escala *e, double p);
double esc_puntos_para(const Escala *e, double nota);    /* -1 si no se alcanza */
int    esc_filas(const Escala *e, double *pts, double *notas, int max);
int    esc_decimales(const Escala *e);
double esc_num(const char *s);                           /* acepta coma o punto; -1e9 si no es número */
void   esc_fmt(double x, int dec, char *out);            /* 4.0 → «4,0» */
int    esc_curso_leer(const Escala *e, const char *texto, Curso *c);   /* «Nombre  48» por línea */

#define ESC_CSV  0
#define ESC_TXT  1
#define ESC_RTF  2
#define ESC_HTML 3
/* Escribe la escala (y el curso si c != NULL y tiene alumnos). 1 = ok. titulo/info pueden ser "". */
int    esc_exportar(const Escala *e, const Curso *c, const char *ruta, int formato, const char *titulo, const char *info);

#endif
