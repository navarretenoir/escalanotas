package io.github.navarretenoir.escalanotas;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.print.PrintAttributes;
import android.print.PrintManager;
import android.util.Base64;
import android.webkit.JavascriptInterface;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStream;

/**
 * Escala de Notas — la interfaz (assets/ui/index.html, la misma de escritorio) en un WebView, sin internet.
 * Puente «EscalaNativa»: guardar exportaciones (Excel, Word, CSV, PNG) e imprimir/guardar en PDF.
 * Desarrollada por Javiera Navarrete.
 */
public class MainActivity extends Activity {
    private static final int GUARDAR = 7;
    private WebView web;
    private byte[] pendiente;

    @SuppressLint({"SetJavaScriptEnabled", "AddJavascriptInterface"})
    @Override
    protected void onCreate(Bundle b) {
        super.onCreate(b);
        web = new WebView(this);
        WebSettings s = web.getSettings();
        s.setJavaScriptEnabled(true);
        s.setDomStorageEnabled(true);                    // recuerda la última escala (localStorage)
        s.setAllowFileAccess(true);
        web.setWebViewClient(new WebViewClient());
        web.addJavascriptInterface(new Puente(), "EscalaNativa");
        setContentView(web);
        web.loadUrl("file:///android_asset/ui/index.html");
        // window.print() → impresión de Android (también «Guardar como PDF»)
        web.setWebViewClient(new WebViewClient() {
            @Override public void onPageFinished(WebView v, String url) {
                v.loadUrl("javascript:window.print=function(){EscalaNativa.imprimir();};");
            }
        });
    }

    @Override
    public void onBackPressed() {
        if (web != null && web.canGoBack()) web.goBack(); else super.onBackPressed();
    }

    private void avisar(boolean ok, String donde) {
        final String js = "javascript:window.escalaGuardado&&escalaGuardado(" + ok + "," + jsTexto(donde) + ")";
        runOnUiThread(new Runnable() { public void run() { web.loadUrl(js); } });
    }

    private static String jsTexto(String s) {
        return "'" + (s == null ? "" : s.replace("\\", "\\\\").replace("'", "\\'")) + "'";
    }

    final class Puente {
        @JavascriptInterface
        public void guardar(String nombre, String mime, String b64) {
            final byte[] datos = Base64.decode(b64, Base64.DEFAULT);
            if (Build.VERSION.SDK_INT >= 19) {
                pendiente = datos;
                final Intent i = dialogoGuardar(nombre, mime);
                runOnUiThread(new Runnable() { public void run() { startActivityForResult(i, GUARDAR); } });
            } else {
                guardarEnDescargas(nombre, datos);
            }
        }

        @JavascriptInterface
        public void imprimir() {
            if (Build.VERSION.SDK_INT >= 19) runOnUiThread(new Runnable() { public void run() { imprimirPagina(); } });
            else avisar(false, "Imprimir requiere Android 4.4");
        }
    }

    @TargetApi(19)
    private Intent dialogoGuardar(String nombre, String mime) {
        Intent i = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        i.addCategory(Intent.CATEGORY_OPENABLE);
        i.setType(mime);
        i.putExtra(Intent.EXTRA_TITLE, nombre);
        return i;
    }

    @TargetApi(19)
    private void imprimirPagina() {
        PrintManager pm = (PrintManager) getSystemService(PRINT_SERVICE);
        String trabajo = "Escala de Notas";
        if (Build.VERSION.SDK_INT >= 21) pm.print(trabajo, web.createPrintDocumentAdapter(trabajo), new PrintAttributes.Builder().build());
        else pm.print(trabajo, web.createPrintDocumentAdapter(), new PrintAttributes.Builder().build());
    }

    private void guardarEnDescargas(String nombre, byte[] datos) {
        try {
            File dir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
            dir.mkdirs();
            File f = new File(dir, nombre);
            FileOutputStream o = new FileOutputStream(f);
            o.write(datos);
            o.close();
            avisar(true, f.getAbsolutePath());
        } catch (Exception e) {
            avisar(false, e.getMessage());
        }
    }

    @Override
    protected void onActivityResult(int pedido, int resultado, Intent datos) {
        super.onActivityResult(pedido, resultado, datos);
        if (pedido != GUARDAR) return;
        if (resultado != RESULT_OK || datos == null || datos.getData() == null || pendiente == null) { pendiente = null; return; }
        Uri u = datos.getData();
        try {
            OutputStream o = getContentResolver().openOutputStream(u);
            o.write(pendiente);
            o.close();
            avisar(true, "el archivo elegido");
        } catch (Exception e) {
            avisar(false, e.getMessage());
        }
        pendiente = null;
    }
}
