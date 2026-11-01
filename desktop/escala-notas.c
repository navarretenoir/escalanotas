/* Escala de Notas — ventana nativa para Linux (GTK 4 + WebKitGTK 6) de la interfaz ui/index.html.
 * Exportar: la página manda {guardar, mime, b64} por el canal «escala» y aquí se guarda con el diálogo del sistema.
 * Imprimir / PDF: window.print() abre el diálogo de impresión de GTK («Imprimir a archivo» = PDF).
 * Desarrollado por Javiera Navarrete. Licencia GPL-3.0-or-later. */
#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <json-glib/json-glib.h>

#ifndef UI_DIR
#define UI_DIR "/usr/share/escala-notas/ui"
#endif

typedef struct { GBytes *datos; WebKitWebView *vista; } Pendiente;

static void js_aviso (WebKitWebView *v, gboolean ok, const char *ruta) {
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
        gboolean ok = g_file_replace_contents (f, d, n, NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL, NULL);
        gchar *ruta = g_file_get_path (f);
        js_aviso (p->vista, ok, ruta);
        g_free (ruta); g_object_unref (f);
    }
    g_bytes_unref (p->datos); g_free (p);
}

static void on_mensaje (WebKitUserContentManager *m, JSCValue *valor, gpointer win) {
    (void) m;
    gchar *txt = jsc_value_to_string (valor);
    JsonParser *jp = json_parser_new ();
    if (json_parser_load_from_data (jp, txt, -1, NULL)) {
        JsonObject *o = json_node_get_object (json_parser_get_root (jp));
        const char *nombre = json_object_get_string_member_with_default (o, "guardar", NULL);
        const char *b64 = json_object_get_string_member_with_default (o, "b64", NULL);
        if (nombre && b64) {
            gsize n; guchar *bin = g_base64_decode (b64, &n);
            Pendiente *p = g_new0 (Pendiente, 1);
            p->datos = g_bytes_new_take (bin, n);
            p->vista = WEBKIT_WEB_VIEW (gtk_window_get_child (GTK_WINDOW (win)));
            GtkFileDialog *dlg = gtk_file_dialog_new ();
            gtk_file_dialog_set_title (dlg, "Guardar escala");
            gtk_file_dialog_set_initial_name (dlg, nombre);
            const char *docs = g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS);
            if (docs) { GFile *d = g_file_new_for_path (docs); gtk_file_dialog_set_initial_folder (dlg, d); g_object_unref (d); }
            gtk_file_dialog_save (dlg, GTK_WINDOW (win), NULL, al_elegir, p);
            g_object_unref (dlg);
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
    gchar *ruta = g_build_filename (dir, "index.html", NULL);
    gchar *uri = g_filename_to_uri (ruta, NULL, NULL);
    webkit_web_view_load_uri (v, uri);
    g_free (uri); g_free (ruta);
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
