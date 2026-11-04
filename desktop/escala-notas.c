/* Escala de Notas — ventana nativa para Linux (GTK 4 + WebKitGTK 6) de la interfaz ui/index.html.
 * Exportar: la página manda {guardar, mime, b64} por el canal «escala» y aquí se guarda con el diálogo del sistema.
 * Imprimir / PDF: window.print() abre el diálogo de impresión de GTK («Imprimir a archivo» = PDF).
 * Desarrollado por Javiera Navarrete. Licencia GPL-3.0-or-later. */
#include <string.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <json-glib/json-glib.h>

#ifndef UI_DIR
#define UI_DIR "/usr/share/escala-notas/ui"
#endif

/* Tope del archivo exportable (decodificado): protege la memoria ante un mensaje desbordado. */
#define MAX_EXPORT (64u * 1024u * 1024u)

typedef struct { GBytes *datos; WebKitWebView *vista; } Pendiente;

static void pendiente_liberar (Pendiente *p) {
    g_bytes_unref (p->datos);
    g_clear_object (&p->vista);
    g_free (p);
}

/* Página mínima de error: evita una ventana en blanco si falta la interfaz o el motor web se cae. */
static void mostrar_error (WebKitWebView *v, const char *titulo, const char *detalle) {
    gchar *t = g_markup_escape_text (titulo, -1), *d = g_markup_escape_text (detalle, -1);
    gchar *html = g_strdup_printf ("<!doctype html><meta charset=utf-8><body style=\"font:16px system-ui;margin:12vh auto;max-width:34em;padding:0 1em\"><h1>%s</h1><p>%s</p></body>", t, d);
    webkit_web_view_load_html (v, html, "about:blank");
    g_free (html); g_free (d); g_free (t);
}

static void cargar_interfaz (WebKitWebView *v, const char *dir) {
    gchar *rel = g_build_filename (dir, "index.html", NULL);
    gchar *ruta = g_canonicalize_filename (rel, NULL);       /* ESCALA_UI puede ser relativa: la URI exige ruta absoluta */
    g_free (rel);
    gchar *uri = g_file_test (ruta, G_FILE_TEST_IS_REGULAR) ? g_filename_to_uri (ruta, NULL, NULL) : NULL;
    if (uri) {
        webkit_web_view_load_uri (v, uri);
        g_free (uri);
    } else {
        gchar *m = g_strdup_printf ("No se encontró la interfaz en «%s». Reinstala la aplicación.", dir);
        mostrar_error (v, "Escala de Notas no pudo iniciar", m);
        g_free (m);
    }
    g_free (ruta);
}

/* Si el proceso web muere (memoria, driver), se recarga en vez de dejar la ventana congelada. */
static void on_proceso_caido (WebKitWebView *v, WebKitWebProcessTerminationReason r, gpointer u) {
    (void) r; (void) u;
    webkit_web_view_reload (v);
}

/* Solo se navega dentro de la propia interfaz: nada sale a internet. */
static gboolean on_decidir (WebKitWebView *v, WebKitPolicyDecision *d, WebKitPolicyDecisionType t, gpointer u) {
    (void) v; (void) u;
    if (t != WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION && t != WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION) return FALSE;
    WebKitNavigationAction *a = webkit_navigation_policy_decision_get_navigation_action (WEBKIT_NAVIGATION_POLICY_DECISION (d));
    const char *uri = webkit_uri_request_get_uri (webkit_navigation_action_get_request (a));
    if (uri && !g_str_has_prefix (uri, "file://") && !g_str_has_prefix (uri, "about:")) { webkit_policy_decision_ignore (d); return TRUE; }
    return FALSE;
}

static void js_aviso (WebKitWebView *v, gboolean ok, const char *ruta) {
    if (!WEBKIT_IS_WEB_VIEW (v)) return;
    gchar *r = g_strescape (ruta ? ruta : "", NULL);
    gchar *js = g_strdup_printf ("window.escalaGuardado && escalaGuardado(%s, \"%s\")", ok ? "true" : "false", r);
    webkit_web_view_evaluate_javascript (v, js, -1, NULL, NULL, NULL, NULL, NULL);
    g_free (js); g_free (r);
}

static void al_elegir (GObject *src, GAsyncResult *res, gpointer u) {
    Pendiente *p = u;
    GFile *f = gtk_file_dialog_save_finish (GTK_FILE_DIALOG (src), res, NULL);
    if (f) {
        gsize n; const char *d = g_bytes_get_data (p->datos, &n);
        GError *err = NULL;
        gboolean ok = g_file_replace_contents (f, d, n, NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL, &err);
        if (!ok) { g_warning ("No se pudo guardar el archivo: %s", err->message); g_clear_error (&err); }
        gchar *ruta = g_file_get_path (f);
        js_aviso (p->vista, ok, ruta);
        g_free (ruta); g_object_unref (f);
    }
    pendiente_liberar (p);
}

static void on_mensaje (WebKitUserContentManager *m, JSCValue *valor, gpointer win) {
    (void) m;
    if (!jsc_value_is_string (valor)) return;
    gchar *txt = jsc_value_to_string (valor);
    JsonParser *jp = json_parser_new ();
    JsonNode *raiz = NULL;
    if (json_parser_load_from_data (jp, txt, -1, NULL) && (raiz = json_parser_get_root (jp)) && JSON_NODE_HOLDS_OBJECT (raiz)) {
        JsonObject *o = json_node_get_object (raiz);
        const char *nombre = json_object_get_string_member_with_default (o, "guardar", NULL);
        const char *b64 = json_object_get_string_member_with_default (o, "b64", NULL);
        WebKitWebView *vista = WEBKIT_WEB_VIEW (gtk_window_get_child (GTK_WINDOW (win)));
        if (nombre && *nombre && b64 && strlen (b64) / 4 * 3 <= MAX_EXPORT) {
            gsize n; guchar *bin = g_base64_decode (b64, &n);
            gchar *base = g_path_get_basename (nombre);      /* el nombre propuesto nunca incluye carpetas */
            Pendiente *p = g_new0 (Pendiente, 1);
            p->datos = g_bytes_new_take (bin, n);
            p->vista = g_object_ref (vista);
            GtkFileDialog *dlg = gtk_file_dialog_new ();
            gtk_file_dialog_set_title (dlg, "Guardar escala");
            gtk_file_dialog_set_initial_name (dlg, base);
            const char *docs = g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS);
            if (docs) { GFile *d = g_file_new_for_path (docs); gtk_file_dialog_set_initial_folder (dlg, d); g_object_unref (d); }
            gtk_file_dialog_save (dlg, GTK_WINDOW (win), NULL, al_elegir, p);
            g_object_unref (dlg); g_free (base);
        } else if (nombre) {
            js_aviso (vista, FALSE, "");
        }
    }
    g_object_unref (jp); g_free (txt);
}

static void activar (GtkApplication *app, gpointer u) {
    (void) u;
    GtkWidget *win = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (win), "Escala de Notas");
    gtk_window_set_default_size (GTK_WINDOW (win), 1180, 820);
    gtk_window_set_icon_name (GTK_WINDOW (win), "io.github.navarretenoir.EscalaNotas");

    WebKitUserContentManager *ucm = webkit_user_content_manager_new ();
    webkit_user_content_manager_register_script_message_handler (ucm, "escala", NULL);
    g_signal_connect (ucm, "script-message-received::escala", G_CALLBACK (on_mensaje), win);
    WebKitWebView *v = WEBKIT_WEB_VIEW (g_object_new (WEBKIT_TYPE_WEB_VIEW, "user-content-manager", ucm, NULL));
    WebKitSettings *s = webkit_web_view_get_settings (v);
    webkit_settings_set_allow_file_access_from_file_urls (s, TRUE);
    webkit_settings_set_enable_developer_extras (s, g_getenv ("ESCALA_DEV") != NULL);
    webkit_settings_set_hardware_acceleration_policy (s, WEBKIT_HARDWARE_ACCELERATION_POLICY_NEVER);

    const char *dir = g_getenv ("ESCALA_UI") ? g_getenv ("ESCALA_UI") : UI_DIR;
    g_signal_connect (v, "decide-policy", G_CALLBACK (on_decidir), NULL);
    g_signal_connect (v, "web-process-terminated", G_CALLBACK (on_proceso_caido), NULL);
    cargar_interfaz (v, dir);
    gtk_window_set_child (GTK_WINDOW (win), GTK_WIDGET (v));
    gtk_window_present (GTK_WINDOW (win));
}

int main (int argc, char **argv) {
    GtkApplication *app = gtk_application_new ("io.github.navarretenoir.EscalaNotas", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect (app, "activate", G_CALLBACK (activar), NULL);
    int r = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    return r;
}
