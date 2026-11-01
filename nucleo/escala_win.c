/* Escala de Notas — versión gráfica clásica: un solo código para Windows 3.x (16 bits) y Windows NT 3.5/3.51/4.0, 9x/Me,
 * 2000–8 (32 bits). Compilador: Open Watcom. Sin dependencias; usa sólo controles estándar (EDIT, BUTTON, LISTBOX).
 * Desarrollada por Javiera Navarrete. Licencia GPL-3.0-or-later. */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "escala.h"

#ifdef __WINDOWS__          /* Win16 */
#define CMD_ID(w, l) ((int)(w))
#else
#define CMD_ID(w, l) ((int)LOWORD(w))
#endif

enum { ID_PMAX = 101, ID_EXIG, ID_NMIN, ID_NAPR, ID_NMAX, ID_PASO, ID_MODO, ID_CALC, ID_LISTA, ID_PTS, ID_PTSR, ID_NOTA, ID_NOTAR,
       ID_CURSO, ID_CSV, ID_TXT, ID_RTF, ID_HTM, ID_TIT, ID_RES, ID_SALIR };

static HWND hw;
static HINSTANCE g_inst;
static Escala E;
static char titulo[64] = "Escala de notas";

static HWND crear(const char *clase, const char *txt, DWORD est, int x, int y, int w, int h, int id) {
    return CreateWindow(clase, txt, WS_CHILD | WS_VISIBLE | est, x, y, w, h, hw, (HMENU)id, g_inst, NULL);
}
static void etiq(const char *t, int x, int y, int w) { crear("STATIC", t, 0, x, y, w, 16, -1); }
static void campo(int id, const char *v, int x, int y, int w) { crear("EDIT", v, WS_BORDER | ES_AUTOHSCROLL, x, y, w, 20, id); }

static double leer_num(int id, double por_defecto) {
    char b[40]; double v;
    GetDlgItemText(hw, id, b, sizeof b);
    v = esc_num(b);
    return v < -1e8 ? por_defecto : v;
}

static int leer_escala(void) {
    E.pmax = leer_num(ID_PMAX, 60);
    E.exig = leer_num(ID_EXIG, 60) / 100.0;
    E.nmin = leer_num(ID_NMIN, 1); E.napr = leer_num(ID_NAPR, 4); E.nmax = leer_num(ID_NMAX, 7);
    E.paso = SendDlgItemMessage(hw, ID_PASO, CB_GETCURSEL, 0, 0) == 1 ? 0.5 : SendDlgItemMessage(hw, ID_PASO, CB_GETCURSEL, 0, 0) == 2 ? 0.25 : 1;
    E.modo = (int)SendDlgItemMessage(hw, ID_MODO, CB_GETCURSEL, 0, 0);
    return esc_valida(&E);
}

static void tabla(void) {
    double *p = (double *)malloc(sizeof(double) * ESC_MAX_FILAS), *n = (double *)malloc(sizeof(double) * ESC_MAX_FILAS);
    char a[24], b[24], l[80];
    int k, i, dec;
    if (!p || !n) return;
    SendDlgItemMessage(hw, ID_LISTA, LB_RESETCONTENT, 0, 0);
    if (!leer_escala()) { SetDlgItemText(hw, ID_RES, "Revisa los datos: la nota minima < aprobacion < maxima."); return; }
    dec = esc_decimales(&E);
    k = esc_filas(&E, p, n, ESC_MAX_FILAS);
    for (i = 0; i < k; i++) {
        esc_fmt(p[i], p[i] == (long)p[i] ? 0 : 2, a); esc_fmt(n[i], dec, b);
        sprintf(l, "%8s   %s%s", a, b, n[i] < E.napr ? "  *" : "");
        SendDlgItemMessage(hw, ID_LISTA, LB_ADDSTRING, 0, (LPARAM)(LPSTR)l);
    }
    esc_fmt(esc_puntos_para(&E, E.napr), 2, a);
    sprintf(l, "Puntos para el %.1f: %s de %.0f", E.napr, a, E.pmax);
    SetDlgItemText(hw, ID_RES, l);
    free(p); free(n);
}

static void consultas(void) {
    char b[40], r[80], a[16];
    double v;
    if (!leer_escala()) return;
    GetDlgItemText(hw, ID_PTS, b, sizeof b); v = esc_num(b);
    if (v > -1e8 && v >= 0 && v <= E.pmax) { esc_fmt(esc_nota(&E, v), esc_decimales(&E), a); sprintf(r, "Nota %s", a); } else r[0] = 0;
    SetDlgItemText(hw, ID_PTSR, r);
    GetDlgItemText(hw, ID_NOTA, b, sizeof b); v = esc_num(b);
    if (v > -1e8) { double q = esc_puntos_para(&E, v); if (q < 0) strcpy(r, "No se alcanza"); else { esc_fmt(q, 2, a); sprintf(r, "Desde %s puntos", a); } } else r[0] = 0;
    SetDlgItemText(hw, ID_NOTAR, r);
}

static void exportar(int fmt) {
    static char txt[8000];
    static Curso cu;
    char ruta[260], *ext[4] = {"csv", "txt", "rtf", "htm"};
    int hay;
    if (!leer_escala()) return;
    GetDlgItemText(hw, ID_TIT, titulo, sizeof titulo);
    GetDlgItemText(hw, ID_CURSO, txt, sizeof txt);
    hay = esc_curso_leer(&E, txt, &cu);
    sprintf(ruta, "escala.%s", ext[fmt]);
    if (esc_exportar(&E, hay ? &cu : NULL, ruta, fmt, titulo, ""))
        { char m[300]; sprintf(m, "Guardado en la carpeta del programa: %s", ruta); MessageBox(hw, m, "Escala de Notas", MB_OK); }
    else MessageBox(hw, "No se pudo guardar el archivo.", "Escala de Notas", MB_OK | MB_ICONSTOP);
}

static void crear_todo(void) {
    int y = 10;
    etiq("Puntaje maximo", 10, y + 2, 100);    campo(ID_PMAX, "60", 115, y, 60);
    etiq("Exigencia %", 190, y + 2, 70);       campo(ID_EXIG, "60", 262, y, 40);
    etiq("Nota minima", 10, y + 32, 100);      campo(ID_NMIN, "1,0", 115, y + 30, 60);
    etiq("Aprobacion", 190, y + 32, 70);       campo(ID_NAPR, "4,0", 262, y + 30, 40);
    etiq("Nota maxima", 10, y + 62, 100);      campo(ID_NMAX, "7,0", 115, y + 60, 60);
    etiq("Intervalo", 190, y + 62, 70);
    crear("COMBOBOX", "", CBS_DROPDOWNLIST | WS_VSCROLL, 262, y + 58, 90, 90, ID_PASO);
    SendDlgItemMessage(hw, ID_PASO, CB_ADDSTRING, 0, (LPARAM)(LPSTR)"1 punto");
    SendDlgItemMessage(hw, ID_PASO, CB_ADDSTRING, 0, (LPARAM)(LPSTR)"1/2 punto");
    SendDlgItemMessage(hw, ID_PASO, CB_ADDSTRING, 0, (LPARAM)(LPSTR)"1/4 punto");
    SendDlgItemMessage(hw, ID_PASO, CB_SETCURSEL, 0, 0);
    etiq("Redondeo", 10, y + 92, 100);
    crear("COMBOBOX", "", CBS_DROPDOWNLIST | WS_VSCROLL, 115, y + 88, 130, 90, ID_MODO);
    SendDlgItemMessage(hw, ID_MODO, CB_ADDSTRING, 0, (LPARAM)(LPSTR)"Aproximar");
    SendDlgItemMessage(hw, ID_MODO, CB_ADDSTRING, 0, (LPARAM)(LPSTR)"Truncar");
    SendDlgItemMessage(hw, ID_MODO, CB_ADDSTRING, 0, (LPARAM)(LPSTR)"2 decimales");
    SendDlgItemMessage(hw, ID_MODO, CB_SETCURSEL, 0, 0);
    crear("BUTTON", "Calcular escala", BS_DEFPUSHBUTTON, 10, y + 122, 135, 26, ID_CALC);
    etiq("Que nota es este puntaje:", 10, y + 160, 170);  campo(ID_PTS, "", 185, y + 156, 50);
    crear("STATIC", "", 0, 245, y + 160, 100, 16, ID_PTSR);
    etiq("Puntos para esta nota:", 10, y + 188, 170);     campo(ID_NOTA, "", 185, y + 184, 50);
    crear("STATIC", "", 0, 245, y + 188, 110, 16, ID_NOTAR);
    crear("STATIC", "", 0, 10, y + 214, 340, 16, ID_RES);
    etiq("Titulo para exportar", 10, y + 240, 130); campo(ID_TIT, "Escala de notas", 145, y + 236, 205);
    crear("BUTTON", "CSV", 0, 10, y + 266, 60, 24, ID_CSV);
    crear("BUTTON", "Texto", 0, 76, y + 266, 60, 24, ID_TXT);
    crear("BUTTON", "Word (RTF)", 0, 142, y + 266, 90, 24, ID_RTF);
    crear("BUTTON", "Web (HTML)", 0, 238, y + 266, 112, 24, ID_HTM);
    crear("LISTBOX", "", WS_BORDER | WS_VSCROLL | LBS_NOTIFY, 370, y, 160, 300, ID_LISTA);
    etiq("Notas del curso: una linea por estudiante, 'Nombre  puntaje'", 10, y + 300, 390);
    crear("EDIT", "", WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN, 10, y + 318, 520, 70, ID_CURSO);
    etiq("Escala de Notas - calculo de notas escolares en Chile. Desarrollada por Javiera Navarrete.", 10, y + 396, 520);
}

LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
    case WM_CREATE: hw = h; esc_defecto(&E); crear_todo(); tabla(); return 0;
    case WM_COMMAND:
        switch (CMD_ID(w, l)) {
        case ID_CALC: tabla(); consultas(); break;
        case ID_PTS: case ID_NOTA: consultas(); break;
        case ID_CSV: exportar(ESC_CSV); break;
        case ID_TXT: exportar(ESC_TXT); break;
        case ID_RTF: exportar(ESC_RTF); break;
        case ID_HTM: exportar(ESC_HTML); break;
        }
        return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(h, m, w, l);
}

int PASCAL WinMain(HINSTANCE hi, HINSTANCE hp, LPSTR cmd, int show) {
    WNDCLASS wc; MSG msg;
    (void)cmd; g_inst = hi;
    if (!hp) {
        memset(&wc, 0, sizeof wc);
        wc.lpfnWndProc = WndProc; wc.hInstance = hi; wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1); wc.lpszClassName = "EscalaNotasWin"; wc.hIcon = LoadIcon(hi, "ICONO");
        if (!wc.hIcon) wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        if (!RegisterClass(&wc)) return 0;
    }
    hw = CreateWindow("EscalaNotasWin", "Escala de Notas", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                      CW_USEDEFAULT, CW_USEDEFAULT, 560, 490, NULL, NULL, hi, NULL);
    ShowWindow(hw, show); UpdateWindow(hw);
    while (GetMessage(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    return (int)msg.wParam;
}
