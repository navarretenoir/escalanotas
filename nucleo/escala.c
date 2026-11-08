/* Escala de Notas — núcleo portable (ver escala.h). Desarrollada por Javiera Navarrete. Licencia GPL-3.0-or-later. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "escala.h"

void esc_defecto(Escala *e) {
    e->pmax = 60; e->exig = 0.6; e->paso = 1; e->nmin = 1.0; e->napr = 4.0; e->nmax = 7.0; e->modo = ESC_APROX1;
}

int esc_valida(const Escala *e) {
    return e->pmax > 0 && e->pmax <= 400 && e->exig > 0 && e->exig < 1 && e->paso > 0 &&
           e->nmin < e->napr && e->napr < e->nmax && e->pmax / e->paso < ESC_MAX_FILAS - 2;
}

int esc_decimales(const Escala *e) { return e->modo == ESC_APROX2 ? 2 : 1; }

static double redondear(double n, int modo) {
    if (modo == ESC_TRUNC1) return floor(n * 10 + 1e-9) / 10;
    if (modo == ESC_APROX2) return floor(n * 100 + 0.5 + 1e-9) / 100;
    return floor(n * 10 + 0.5 + 1e-9) / 10;          /* aproximar: 3,95 → 4,0 */
}

double esc_nota(const Escala *e, double p) {
    double papr = e->pmax * e->exig, n;
    n = p < papr ? e->nmin + (e->napr - e->nmin) * p / papr
                 : e->napr + (e->nmax - e->napr) * (p - papr) / (e->pmax - papr);
    n = redondear(n, e->modo);
    if (n < e->nmin) n = e->nmin;
    if (n > e->nmax) n = e->nmax;
    return n;
}

double esc_puntos_para(const Escala *e, double nota) {
    double p;
    if (nota <= e->nmin) return 0;
    if (nota > e->nmax) return -1;
    for (p = 0; p <= e->pmax + 1e-9; p += e->paso)
        if (esc_nota(e, p) >= nota - 1e-9) return floor(p * 100 + 0.5) / 100;
    return -1;
}

int esc_filas(const Escala *e, double *pts, double *notas, int max) {
    int n = (int)floor(e->pmax / e->paso + 1e-9), i, k = 0;
    for (i = 0; i <= n && k < max; i++, k++) {
        pts[k] = floor(i * e->paso * 100 + 0.5) / 100;
        notas[k] = esc_nota(e, pts[k]);
    }
    if (k > 0 && k < max && pts[k - 1] < e->pmax) { pts[k] = e->pmax; notas[k] = esc_nota(e, e->pmax); k++; }
    return k;
}

double esc_num(const char *s) {
    char b[32]; int i = 0; char *fin;
    double v;
    while (*s == ' ' || *s == '\t') s++;
    for (; *s && i < 31; s++) b[i++] = (*s == ',') ? '.' : *s;
    b[i] = 0;
    while (i > 0 && (b[i - 1] == ' ' || b[i - 1] == '\t' || b[i - 1] == '\r' || b[i - 1] == '\n')) b[--i] = 0;
    if (!b[0]) return -1e9;
    v = strtod(b, &fin);
    return *fin ? -1e9 : v;
}

void esc_fmt(double x, int dec, char *out) {
    char *p;
    sprintf(out, dec == 0 ? "%.0f" : dec == 2 ? "%.2f" : dec == 3 ? "%.3f" : "%.1f", x);
    for (p = out; *p; p++) if (*p == '.') *p = ',';
}

static void fmt_pts(double p, char *out) {           /* 36 → «36», 36,5 → «36,5», 36,25 → «36,25» */
    char *q;
    sprintf(out, "%.2f", p);
    q = out + strlen(out) - 1;
    while (*q == '0') *q-- = 0;
    if (*q == '.') *q = 0;
    for (q = out; *q; q++) if (*q == '.') *q = ',';
}

/* Celda de texto para CSV (separador «;»): entre comillas si trae ; " o saltos de línea, y con un apóstrofo delante
 * si empieza con = + - @ (si no, Excel/Calc lo ejecutaría como fórmula al abrir el archivo). */
static void csv_txt(FILE *f, const char *s) {
    int comillas = strpbrk(s, ";\"\r\n") != NULL || *s == '=' || *s == '+' || *s == '-' || *s == '@';
    if (comillas) fputc('"', f);
    if (*s == '=' || *s == '+' || *s == '-' || *s == '@') fputc('\'', f);
    for (; *s; s++) { if (*s == '"') fputc('"', f); fputc(*s == '\r' || *s == '\n' ? ' ' : *s, f); }
    if (comillas) fputc('"', f);
}

static int cmp_d(const void *a, const void *b) {
    double x = *(const double *)a, y = *(const double *)b;
    return x < y ? -1 : x > y;
}

/* Copia el nombre recortado a 39 bytes sin partir un carácter UTF-8 (un «ñ» cortado a la mitad deja el archivo exportado inválido). */
static void copiar_nombre(char *dst, const char *src) {
    int n = 0;
    while (src[n] && n < 39) n++;
    if (src[n]) while (n > 0 && ((unsigned char)src[n] & 0xC0) == 0x80) n--;   /* retrocede hasta el inicio del carácter */
    memcpy(dst, src, (size_t)n);
    dst[n] = 0;
}

int esc_curso_leer(const Escala *e, const char *texto, Curso *c) {
    static double ord[ESC_MAX_CURSO];
    const char *l = texto;
    double suma = 0;
    int i;
    memset(c, 0, sizeof *c);
    while (*l && c->total < ESC_MAX_CURSO) {
        char lin[96]; int n = 0, fin, ini;
        double p;
        while (*l && *l != '\n' && n < 95) lin[n++] = *l++;
        while (*l && *l != '\n') l++;
        if (*l) l++;
        lin[n] = 0;
        while (n > 0 && (lin[n - 1] == '\r' || lin[n - 1] == ' ' || lin[n - 1] == '\t')) lin[--n] = 0;
        if (!n) continue;
        /* el puntaje es el último número de la línea; el resto es el nombre */
        fin = n;
        ini = fin;
        while (ini > 0 && ((lin[ini - 1] >= '0' && lin[ini - 1] <= '9') || lin[ini - 1] == ',' || lin[ini - 1] == '.')) ini--;
        if (ini == fin) { c->ignoradas++; continue; }
        p = esc_num(lin + ini);
        if (p < -1e8) { c->ignoradas++; continue; }
        while (ini > 0 && (lin[ini - 1] == ' ' || lin[ini - 1] == '\t' || lin[ini - 1] == ';' || lin[ini - 1] == '|')) ini--;
        lin[ini] = 0;
        if (p < 0) { p = 0; c->ajustados++; }
        if (p > e->pmax) { p = e->pmax; c->ajustados++; }
        if (lin[0]) copiar_nombre(c->a[c->total].nombre, lin);
        else sprintf(c->a[c->total].nombre, "Estudiante %d", c->total + 1);
        c->a[c->total].p = p;
        c->a[c->total].n = esc_nota(e, p);
        c->total++;
    }
    if (!c->total) return 0;
    for (i = 0; i < c->total; i++) {
        double n = c->a[i].n; int b = (int)floor(n) - 1;
        ord[i] = n; suma += n;
        if (n >= e->napr) c->aprobados++;
        if (b < 0) b = 0;
        if (b > 5) b = 5;
        c->bins[b]++;
    }
    qsort(ord, c->total, sizeof(double), cmp_d);
    c->promedio = suma / c->total;
    c->mediana = c->total % 2 ? ord[c->total / 2] : (ord[c->total / 2 - 1] + ord[c->total / 2]) / 2;
    c->nota_baja = ord[0]; c->nota_alta = ord[c->total - 1];
    return c->total;
}

/* ------------------------------------------------------------------ exportación */
static void rtf_txt(FILE *f, const char *s) {        /* UTF-8 → RTF con \u (Word 97+) */
    const unsigned char *p = (const unsigned char *)s;
    while (*p) {
        unsigned cp = *p;
        if (cp < 0x80) { if (cp == '\\' || cp == '{' || cp == '}') fputc('\\', f); fputc((int)cp, f); p++; continue; }
        if ((cp & 0xE0) == 0xC0 && p[1]) { cp = ((cp & 0x1F) << 6) | (p[1] & 0x3F); p += 2; }
        else if ((cp & 0xF0) == 0xE0 && p[1] && p[2]) { cp = ((cp & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F); p += 3; }
        else { p++; continue; }
        fprintf(f, "\\u%d?", cp > 32767 ? (int)cp - 65536 : (int)cp);
    }
}
static void html_txt(FILE *f, const char *s) {
    for (; *s; s++) {
        if (*s == '<') fputs("&lt;", f); else if (*s == '>') fputs("&gt;", f); else if (*s == '&') fputs("&amp;", f); else fputc(*s, f);
    }
}

int esc_exportar(const Escala *e, const Curso *c, const char *ruta, int formato, const char *titulo, const char *info) {
    double *pts, *notas;
    char a[24], b[24], linea[160];
    int n, i, dec = esc_decimales(e), hay_curso = c && c->total > 0;
    FILE *f = fopen(ruta, formato == ESC_RTF ? "wb" : "w");
    if (!f) return 0;
    pts = (double *)malloc(sizeof(double) * ESC_MAX_FILAS); notas = (double *)malloc(sizeof(double) * ESC_MAX_FILAS);
    if (!pts || !notas) { if (pts) free(pts); if (notas) free(notas); fclose(f); return 0; }
    n = esc_filas(e, pts, notas, ESC_MAX_FILAS);
    if (!titulo || !*titulo) titulo = "Escala de notas";
    sprintf(linea, "Puntaje m\xC3\xA1ximo %s \xC2\xB7 exigencia %.0f %% \xC2\xB7 notas ", "", e->exig * 100);
    {   /* resumen de parámetros */
        char pm[16], mn[8], mx[8];
        fmt_pts(e->pmax, pm); esc_fmt(e->nmin, 1, mn); esc_fmt(e->nmax, 1, mx);
        sprintf(linea, "Puntaje m\xC3\xA1ximo %s \xC2\xB7 exigencia %.0f %% \xC2\xB7 notas %s a %s", pm, e->exig * 100, mn, mx);
    }
    if (formato == ESC_CSV) {
        fputs("\xEF\xBB\xBF", f);
        csv_txt(f, titulo); fputc('\n', f);
        if (info && *info) { csv_txt(f, info); fputs(" \xC2\xB7 ", f); }
        csv_txt(f, linea); fputs("\n\nPuntaje;% de logro;Nota\n", f);
        for (i = 0; i < n; i++) { fmt_pts(pts[i], a); esc_fmt(notas[i], dec, b); fprintf(f, "%s;%.1f;%s\n", a, pts[i] / e->pmax * 100, b); }
        if (hay_curso) {
            fputs("\nNotas del curso\nEstudiante;Puntaje;Nota\n", f);
            for (i = 0; i < c->total; i++) { fmt_pts(c->a[i].p, a); esc_fmt(c->a[i].n, dec, b); csv_txt(f, c->a[i].nombre); fprintf(f, ";%s;%s\n", a, b); }
        }
    } else if (formato == ESC_TXT) {
        fprintf(f, "%s\n%s%s%s\n\n", titulo, info && *info ? info : "", info && *info ? " \xC2\xB7 " : "", linea);
        fputs("Puntaje   Nota\n--------  ----\n", f);
        for (i = 0; i < n; i++) { fmt_pts(pts[i], a); esc_fmt(notas[i], dec, b); fprintf(f, "%8s  %s%s\n", a, b, notas[i] < e->napr ? "  (insuficiente)" : ""); }
        if (hay_curso) {
            fputs("\nNotas del curso\n", f);
            for (i = 0; i < c->total; i++) { fmt_pts(c->a[i].p, a); esc_fmt(c->a[i].n, dec, b); fprintf(f, "%-38s %8s  %s\n", c->a[i].nombre, a, b); }
            esc_fmt(c->promedio, 2, a); fprintf(f, "\nPromedio %s \xC2\xB7 %d de %d aprobados\n", a, c->aprobados, c->total);
        }
    } else if (formato == ESC_RTF) {
        fputs("{\\rtf1\\ansi\\ansicpg1252\\deff0{\\fonttbl{\\f0 Arial;}}{\\colortbl;\\red192\\green0\\blue0;\\red11\\green58\\blue140;}\n\\f0\\fs32\\b ", f);
        rtf_txt(f, titulo); fputs("\\b0\\par\\fs20 ", f);
        if (info && *info) { rtf_txt(f, info); fputs(" \\u183? ", f); }
        rtf_txt(f, linea); fputs("\\par\\par\n", f);
        for (i = 0; i < n; i++) {
            fmt_pts(pts[i], a); esc_fmt(notas[i], dec, b);
            fprintf(f, "\\trowd\\cellx1800\\cellx3600 %s\\cell \\cf%d\\b %s\\b0\\cf0\\cell\\row\n", a, notas[i] < e->napr ? 1 : 2, b);
        }
        if (hay_curso) {
            fputs("\\par\\b Notas del curso\\b0\\par\n", f);
            for (i = 0; i < c->total; i++) {
                fmt_pts(c->a[i].p, a); esc_fmt(c->a[i].n, dec, b);
                fputs("\\trowd\\cellx4200\\cellx5600\\cellx7000 ", f); rtf_txt(f, c->a[i].nombre);
                fprintf(f, "\\cell %s\\cell \\cf%d %s\\cf0\\cell\\row\n", a, c->a[i].n < e->napr ? 1 : 2, b);
            }
        }
        fputs("}\n", f);
    } else {
        fputs("<!DOCTYPE html><html lang=\"es-CL\"><head><meta charset=\"utf-8\"><title>", f); html_txt(f, titulo);
        fputs("</title><style>body{font:14px Arial,sans-serif;margin:24px}td,th{padding:2px 10px;text-align:right;border-bottom:1px solid #ddd}"
              ".r{color:#c00;font-weight:bold}.a{color:#0b3a8c;font-weight:bold}table{border-collapse:collapse;margin-bottom:20px}</style></head><body><h1>", f);
        html_txt(f, titulo); fputs("</h1><p>", f);
        if (info && *info) { html_txt(f, info); fputs(" \xC2\xB7 ", f); }
        html_txt(f, linea); fputs("</p><table><tr><th>Puntaje</th><th>% de logro</th><th>Nota</th></tr>\n", f);
        for (i = 0; i < n; i++) { fmt_pts(pts[i], a); esc_fmt(notas[i], dec, b); fprintf(f, "<tr><td>%s</td><td>%.1f%%</td><td class=\"%s\">%s</td></tr>\n", a, pts[i] / e->pmax * 100, notas[i] < e->napr ? "r" : "a", b); }
        fputs("</table>", f);
        if (hay_curso) {
            fputs("<h2>Notas del curso</h2><table><tr><th>Estudiante</th><th>Puntaje</th><th>Nota</th></tr>\n", f);
            for (i = 0; i < c->total; i++) {
                fmt_pts(c->a[i].p, a); esc_fmt(c->a[i].n, dec, b);
                fputs("<tr><td style=\"text-align:left\">", f); html_txt(f, c->a[i].nombre);
                fprintf(f, "</td><td>%s</td><td class=\"%s\">%s</td></tr>\n", a, c->a[i].n < e->napr ? "r" : "a", b);
            }
            fputs("</table>", f);
        }
        fputs("<p style=\"color:#888\">Escala de Notas \xE2\x80\x94 c\xC3\xA1lculo de notas escolares en Chile. Desarrollada por Javiera Navarrete.</p></body></html>\n", f);
    }
    free(pts); free(notas);
    if (ferror(f)) { fclose(f); return 0; }          /* disco lleno o medio retirado: no se declara éxito */
    return fclose(f) == 0;
}
