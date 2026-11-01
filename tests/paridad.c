/* Imprime nota por puntaje para una rejilla de configuraciones (lo compara paridad.js). */
#include <stdio.h>
#include "../nucleo/escala.h"
int main(void) {
    static const double pm[] = {10, 20, 25, 30, 45, 50, 60, 100, 120}, ex[] = {40, 50, 60, 70}, pa[] = {1, 0.5, 0.25};
    int a, b, c, m, k, n; static double p[ESC_MAX_FILAS], no[ESC_MAX_FILAS];
    for (a = 0; a < 9; a++) for (b = 0; b < 4; b++) for (c = 0; c < 3; c++) for (m = 0; m < 3; m++) {
        Escala e; esc_defecto(&e); e.pmax = pm[a]; e.exig = ex[b] / 100; e.paso = pa[c]; e.modo = m;
        n = esc_filas(&e, p, no, ESC_MAX_FILAS);
        printf("%g/%g/%g/%d:", pm[a], ex[b], pa[c], m);
        for (k = 0; k < n; k++) printf("%.2f,", no[k]);
        printf("|%.2f\n", esc_puntos_para(&e, 5.5));
    }
    return 0;
}
