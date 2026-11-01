/* Escala de Notas para Windows — configuración del anfitrión WebView2 (webhost.c). Desarrollada por Javiera Navarrete. */
#define APP_NOMBRE          L"Escala de Notas"
#define APP_CLASE           L"EscalaNotasJavieraNavarrete"
#define APP_AGENTE          "Escala-Notas-Windows/1.0 (Javiera Navarrete)"
#define APP_HOST_VIRTUAL    L"escala.local"
#define APP_CARPETA_UI      L"ui"
#define APP_INICIO          L"https://escala.local/index.html"
#define APP_CARPETA_DATOS   L"EscalaNotas"
#define APP_PUENTE          1
#define APP_GUARDAR         1
#define APP_VENTANAS_DENTRO 0
#define APP_MENU_CONTEXTUAL FALSE
#define APP_ANCHO           1180
#define APP_ALTO            820
#define APP_COLOR_FONDO     RGB(0xE9, 0xE7, 0xE1)
#define APP_ACERCA          L"Escala de Notas 1.0 para Windows\n\nHecha para el cálculo de notas escolares en Chile (1,0 a 7,0).\n" \
                            L"Desarrollada por Javiera Navarrete. Licencia GPL-3.0-or-later.\n\nFuncionamiento sin internet."
static const WCHAR *APP_FUERA[] = {L"http://", L"https://github.com/", L"https://gitlab.com/", NULL};
static const WCHAR *APP_PERMITIDOS[] = {NULL};
