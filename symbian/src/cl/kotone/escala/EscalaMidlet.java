package cl.kotone.escala;

import javax.microedition.lcdui.Display;
import javax.microedition.lcdui.Graphics;
import javax.microedition.lcdui.TextField;
import javax.microedition.midlet.MIDlet;
import cl.kotone.ui.*;

/**
 * Escala de Notas para Symbian (Java ME): interfaz táctil propia, escala chilena 1,0–7,0 por tramos con cualquier exigencia y puntaje.
 * Sin internet. Desarrollada por Javiera Navarrete. GPL-3.0-or-later.
 */
public class EscalaMidlet extends MIDlet implements Pantalla.Salida {
    Pantalla pan; final Escala e = new Escala();
    String sPmax = "60", sExig = "60", sNmin = "1,0", sNapr = "4,0", sNmax = "7,0", texto = ""; int paso = 0, redondeo = 0;
    static final String[] PASOS = { "1 punto", "½ punto", "¼ punto" }, REDONDEOS = { "1 decimal", "Truncar", "2 decimales" };

    protected void startApp() {
        if (pan != null) { pan.mostrar(); return; }
        Ui.tema(0x3F83E8, 0x1E4FB5, 0x1D4FB0, 0xFFB13B, 0xEEF2FA, 0x0F2352);
        pan = new Pantalla(Display.getDisplay(this), this);
        String[] c = Pref.leer("escala-v2");
        if (c != null && c.length >= 8) { sPmax = c[0]; sExig = c[1]; sNmin = c[2]; sNapr = c[3]; sNmax = c[4]; try { paso = Integer.parseInt(c[5]); redondeo = Integer.parseInt(c[6]); } catch (Exception x) { } texto = c[7]; }
        pan.abrir(new Ajustes());
        pan.mostrar();
    }
    protected void pauseApp() { }
    protected void destroyApp(boolean u) { guardar(); }
    public void salir() { guardar(); destroyApp(true); notifyDestroyed(); }
    void guardar() { Pref.guardar("escala-v2", new String[] { sPmax, sExig, sNmin, sNapr, sNmax, "" + paso, "" + redondeo, texto }); }

    /** Lee los ajustes; devuelve un mensaje de error o null si todo bien. */
    String leer() {
        e.pmax = Escala.num(sPmax); e.exig = Escala.num(sExig) / 100; e.paso = paso == 1 ? 0.5 : paso == 2 ? 0.25 : 1;
        e.nmin = Escala.num(sNmin); e.napr = Escala.num(sNapr); e.nmax = Escala.num(sNmax); e.modo = redondeo; e.preparar();
        if (Double.isNaN(e.pmax) || Double.isNaN(e.exig) || Double.isNaN(e.nmin) || Double.isNaN(e.napr) || Double.isNaN(e.nmax)) return "Revisa los números: hay un campo vacío o con letras.";
        if (!e.ok()) return "Datos inválidos: puntaje entre 1 y 400, exigencia entre 1 y 99 %, y nota mínima < aprobación < máxima.";
        guardar(); return null;
    }

    // ============================================================ Ajustes (inicio)
    final class Ajustes extends Vista {
        final Zonas z = new Zonas();
        Ajustes() { titulo = "Escala de notas"; barra = new int[] { Ui.I_LIBRO, Ui.I_CALC, Ui.I_MENU, Ui.I_INFO }; barraTxt = new String[] { "Tabla", "Consultar", "Curso", "Info" }; }
        public void accion(int i) { if (i == 3) { pan.abrir(new Acerca()); return; } ir(i); }
        void ir(int i) { String m = leer(); if (m != null) { pan.aviso(m); return; } pan.abrir(i == 0 ? (Vista) new Tabla() : i == 1 ? (Vista) new Consulta() : (Vista) new Curso()); }
        public void pintar(Graphics g, int w, int y0, int y1) {
            z.limpiar(); int m = Ui.k(14), y = m, aw = w - 2 * m;
            Ui.tarjeta(g, m, y, aw, Ui.k(74));
            Ui.centrado(g, "Nota 4,0 desde", w / 2, y + Ui.k(8), Ui.fS, Ui.TEXTO2);
            double pa = Escala.num(sPmax) * Escala.num(sExig) / 100;
            Ui.centrado(g, Double.isNaN(pa) ? "—" : Escala.pts(pa) + " de " + sPmax + " puntos", w / 2, y + Ui.k(26), Ui.fL, Ui.ACENTO);
            Ui.centrado(g, "Exigencia " + sExig + " %  ·  escala " + sNmin + " a " + sNmax, w / 2, y + Ui.k(52), Ui.fS, Ui.TEXTO2);
            y += Ui.k(74) + m;
            int fh = Ui.altoCampo(), mitad = (aw - m) / 2;
            Ui.campo(g, m, y, mitad, fh, "Puntaje máximo", sPmax); z.agregar(m, y, mitad, fh, 1);
            Ui.campo(g, m + mitad + m, y, mitad, fh, "Exigencia %", sExig); z.agregar(m + mitad + m, y, mitad, fh, 2); y += fh + m;
            int t = (aw - 2 * m) / 3;
            Ui.campo(g, m, y, t, fh, "Nota mínima", sNmin); z.agregar(m, y, t, fh, 3);
            Ui.campo(g, m + t + m, y, t, fh, "Aprobación", sNapr); z.agregar(m + t + m, y, t, fh, 4);
            Ui.campo(g, m + 2 * (t + m), y, t, fh, "Nota máxima", sNmax); z.agregar(m + 2 * (t + m), y, t, fh, 5); y += fh + m;
            y = chips(g, m, y, w, "Intervalo de puntaje", PASOS, paso, 10);
            y = chips(g, m, y, w, "Redondeo de la nota", REDONDEOS, redondeo, 20);
            int bh = Ui.k(46);
            Ui.boton(g, m, y, aw, bh, "Ver tabla de notas", 0); z.agregar(m, y, aw, bh, 30); y += bh + m / 2 + Ui.k(4);
            Ui.boton(g, m, y, (aw - m) / 2, bh, "Consultar", 2); z.agregar(m, y, (aw - m) / 2, bh, 31);
            Ui.boton(g, m + (aw - m) / 2 + m, y, (aw - m) / 2, bh, "Curso", 2); z.agregar(m + (aw - m) / 2 + m, y, (aw - m) / 2, bh, 32); y += bh + m;
            Ui.centrado(g, "Funciona sin internet", w / 2, y, Ui.fS, Ui.TEXTO2); y += Ui.fS.getHeight() + m;
            alto = y;
        }
        int chips(Graphics g, int m, int y, int w, String et, String[] op, int sel, int base) {
            g.setFont(Ui.fS); g.setColor(Ui.TEXTO2); g.drawString(et, m, y, Graphics.TOP | Graphics.LEFT); y += Ui.fS.getHeight() + Ui.k(4);
            int x = m, ch = Ui.altoChip(Ui.fN);
            for (int i = 0; i < op.length; i++) {
                int cw = Ui.fN.stringWidth(op[i]) + Ui.k(22);
                if (x + cw > w - m && x > m) { x = m; y += ch + Ui.k(6); }
                Ui.chip(g, x, y, op[i], i == sel, Ui.fN); z.agregar(x, y, cw, ch, base + i); x += cw + Ui.k(8);
            }
            return y + ch + m;
        }
        public void toque(int x, int y) {
            int id = z.en(x, y);
            if (id >= 1 && id <= 5) { String[] et = { "", "Puntaje máximo", "Exigencia (%)", "Nota mínima", "Nota de aprobación", "Nota máxima" }; String[] v = { "", sPmax, sExig, sNmin, sNapr, sNmax }; pan.pedirTexto(this, id, et[id], v[id], 8, TextField.DECIMAL); }
            else if (id >= 10 && id < 13) paso = id - 10;
            else if (id >= 20 && id < 23) redondeo = id - 20;
            else if (id >= 30 && id <= 32) ir(id - 30);
        }
        public void entrada(int id, String t) {
            t = t.trim(); switch (id) { case 1: sPmax = t; break; case 2: sExig = t; break; case 3: sNmin = t; break; case 4: sNapr = t; break; case 5: sNmax = t; break; }
            guardar();
        }
    }

    // ============================================================ Tabla
    final class Tabla extends Vista {
        final int n, d; final String[] fp, fn, fl; final boolean[] ap; final int marca;
        Tabla() {
            titulo = "Tabla · 0–" + Escala.pts(e.pmax) + " · " + Escala.coma(e.exig * 100, 0) + " %"; d = e.dec(); n = e.filas();
            fp = new String[n]; fn = new String[n]; fl = new String[n]; ap = new boolean[n]; int mk = -1;
            for (int i = 0; i < n; i++) { double p = e.puntoFila(i), nt = e.nota(p); fp[i] = Escala.pts(p); fn[i] = Escala.coma(nt, d); fl[i] = Escala.coma(p / e.pmax * 100, 0) + " %"; ap[i] = nt >= e.napr - 1e-9; if (ap[i] && mk < 0) mk = i; }
            marca = mk;
        }
        public void pintar(Graphics g, int w, int y0, int y1) {
            int rh = Ui.k(40), hd = Ui.k(34);
            g.setColor(Ui.mezcla(Ui.ACENTO, 0xFFFFFF, 190)); g.fillRect(0, 0, w, hd);
            g.setFont(Ui.fB); g.setColor(Ui.ACENTO); int ty = (hd - Ui.fB.getHeight()) / 2;
            g.drawString("Puntos", Ui.k(16), ty, Graphics.TOP | Graphics.LEFT); Ui.centrado(g, "Nota", w * 55 / 100, ty, Ui.fB, Ui.ACENTO); Ui.derecha(g, "Logro", w - Ui.k(16), ty, Ui.fB, Ui.ACENTO);
            int a = (y0 - hd) / rh; if (a < 0) a = 0; int b = (y1 - hd) / rh + 1; if (b > n) b = n;
            for (int i = a; i < b; i++) {
                int y = hd + i * rh;
                g.setColor(i == marca ? 0xD9F2E3 : (i & 1) == 0 ? 0xFFFFFF : 0xF3F6FB); g.fillRect(0, y, w, rh);
                int ty2 = y + (rh - Ui.fN.getHeight()) / 2;
                g.setFont(Ui.fN); g.setColor(Ui.TEXTO); g.drawString(fp[i], Ui.k(16), ty2, Graphics.TOP | Graphics.LEFT);
                Ui.centrado(g, fn[i], w * 55 / 100, y + (rh - Ui.fB.getHeight()) / 2, Ui.fB, ap[i] ? 0x1F7A43 : Ui.ROJO);
                Ui.derecha(g, fl[i], w - Ui.k(16), ty2, Ui.fN, Ui.TEXTO2);
                if (i == marca) { g.setColor(0x1F7A43); g.fillRect(0, y, Ui.k(4), rh); }
            }
            alto = hd + n * rh + Ui.k(10);
        }
    }

    // ============================================================ Consultar
    final class Consulta extends Vista {
        String qp = "", qn = ""; final Zonas z = new Zonas();
        Consulta() { titulo = "Consultar"; }
        public void pintar(Graphics g, int w, int y0, int y1) {
            z.limpiar(); int m = Ui.k(14), y = m, aw = w - 2 * m, fh = Ui.altoCampo();
            Ui.campo(g, m, y, aw, fh, "¿Qué nota es este puntaje?", qp); z.agregar(m, y, aw, fh, 1); y += fh + Ui.k(8);
            double p = Escala.num(qp); String r1;
            if (qp.length() == 0 || Double.isNaN(p)) r1 = "Escribe un puntaje (0 a " + Escala.pts(e.pmax) + ")"; else if (p < 0 || p > e.pmax) r1 = "Fuera de rango (0 a " + Escala.pts(e.pmax) + ")"; else r1 = null;
            y = resultado(g, m, y, aw, r1 == null ? Escala.coma(e.nota(p), e.dec()) : "—", r1 == null ? Escala.coma(p / e.pmax * 100, 1) + " % de logro" : r1, r1 == null ? e.nota(p) >= e.napr - 1e-9 : false);
            y += m;
            Ui.campo(g, m, y, aw, fh, "¿Cuántos puntos pide esta nota?", qn); z.agregar(m, y, aw, fh, 2); y += fh + Ui.k(8);
            double n = Escala.num(qn); String r2 = null, big = "—";
            if (qn.length() == 0 || Double.isNaN(n)) r2 = "Escribe una nota (" + Escala.coma(e.nmin, 1) + " a " + Escala.coma(e.nmax, 1) + ")";
            else { double q = e.puntosPara(n); if (q < 0) r2 = "Esa nota no se alcanza con esta escala"; else { big = Escala.pts(q); r2 = "puntos de " + Escala.pts(e.pmax) + " como mínimo"; } }
            y = resultado(g, m, y, aw, big, r2, !big.equals("—"));
            alto = y + m;
        }
        int resultado(Graphics g, int m, int y, int aw, String grande, String chico, boolean verde) {
            int h = Ui.k(78); Ui.tarjeta(g, m, y, aw, h);
            Ui.centrado(g, grande, m + aw / 2, y + Ui.k(8), Ui.fL, grande.equals("—") ? Ui.TEXTO2 : verde ? 0x1F7A43 : Ui.ROJO);
            String[] l = Ui.envolver(chico, Ui.fS, aw - Ui.k(20)); int yy = y + Ui.k(8) + Ui.fL.getHeight() + Ui.k(6);
            for (int i = 0; i < l.length && i < 2; i++) Ui.centrado(g, l[i], m + aw / 2, yy + i * (Ui.fS.getHeight() + 2), Ui.fS, Ui.TEXTO2);
            return y + h;
        }
        public void toque(int x, int y) { int id = z.en(x, y); if (id == 1) pan.pedirTexto(this, 1, "Puntaje", qp, 8, TextField.DECIMAL); else if (id == 2) pan.pedirTexto(this, 2, "Nota", qn, 5, TextField.DECIMAL); }
        public void entrada(int id, String t) { if (id == 1) qp = t.trim(); else qn = t.trim(); }
    }

    // ============================================================ Curso
    final class Curso extends Vista {
        final Zonas z = new Zonas(); String[] lin; int lw = -1;
        Curso() { titulo = "Notas del curso"; barra = new int[] { Ui.I_CHECK }; barraTxt = new String[] { "Calcular" }; }
        public void invalidar() { lw = -1; }
        public void accion(int i) { calcular(); }
        void calcular() { guardar(); Escala.Curso c = e.curso(texto); pan.abrir(new Resultado(c)); }
        public void pintar(Graphics g, int w, int y0, int y1) {
            z.limpiar(); int m = Ui.k(14), aw = w - 2 * m, y = m;
            String ayuda = "Escribe un estudiante por línea: nombre y puntaje. Ejemplo:\nAna Pérez 48\nLuis Soto 35,5\nUna línea sólo con el puntaje se llama «Estudiante N».";
            String[] a = Ui.envolver(ayuda, Ui.fS, aw); y += Ui.lineas(g, a, m, y, Ui.fS, Ui.TEXTO2, 20) + Ui.k(8);
            int bh = Ui.k(190); Ui.tarjeta(g, m, y, aw, bh); z.agregar(m, y, aw, bh, 1);
            if (lw != aw) { lin = Ui.envolver(texto.length() == 0 ? "Toca aquí para escribir las notas del curso" : texto, Ui.fN, aw - Ui.k(20)); lw = aw; }
            g.setClip(m, y, aw, bh); // evita desbordes visuales
            int yy = y + Ui.k(10), n = (bh - Ui.k(20)) / (Ui.fN.getHeight() + Ui.k(2)); if (n > lin.length) n = lin.length;
            Ui.lineas(g, lin, m + Ui.k(10), yy, Ui.fN, texto.length() == 0 ? 0xA5B0AE : Ui.TEXTO, n);
            g.setClip(0, y0, w, y1 - y0); y += bh + Ui.k(10);
            Ui.boton(g, m, y, aw, Ui.k(46), "Calcular", 0); z.agregar(m, y, aw, Ui.k(46), 2); y += Ui.k(46) + Ui.k(10);
            Ui.boton(g, m, y, aw, Ui.k(42), "Borrar la lista", 2); z.agregar(m, y, aw, Ui.k(42), 3); y += Ui.k(42) + m;
            alto = y;
        }
        public void toque(int x, int y) { int id = z.en(x, y); if (id == 1) pan.pedirTexto(this, 1, "Curso: nombre y puntaje", texto, 4000, TextField.ANY); else if (id == 2) calcular(); else if (id == 3) { texto = ""; lw = -1; guardar(); } }
        public void entrada(int id, String t) { texto = t; lw = -1; guardar(); }
    }
    final class Resultado extends Vista {
        final Escala.Curso c; final int d; final int n;
        Resultado(Escala.Curso c) { this.c = c; titulo = "Resultados"; d = e.dec(); n = c.alumnos.size(); }
        public void pintar(Graphics g, int w, int y0, int y1) {
            int m = Ui.k(14), aw = w - 2 * m, y = m;
            if (n == 0) { String[] l = Ui.envolver("Ninguna línea tiene un puntaje válido. Escribe el nombre y luego el puntaje, por ejemplo «Ana Pérez 48».", Ui.fN, aw); Ui.lineas(g, l, m, y, Ui.fN, Ui.TEXTO, 10); alto = y + 200; return; }
            int th = Ui.k(86); Ui.tarjeta(g, m, y, aw, th); int cw = aw / 3;
            dato(g, m, y, cw, Escala.coma(Math.floor(c.promedio * 100 + 0.5) / 100, 2), "Promedio"); dato(g, m + cw, y, cw, Escala.coma(c.mediana, d), "Mediana");
            dato(g, m + 2 * cw, y, cw, c.aprobados + "/" + n, "Aprobados " + (int) Math.floor(c.aprobados * 100.0 / n + 0.5) + " %"); y += th + m;
            // distribución
            int gh = Ui.k(120); Ui.tarjeta(g, m, y, aw, gh + Ui.k(34)); Ui.centrado(g, "Distribución de notas", m + aw / 2, y + Ui.k(6), Ui.fS, Ui.TEXTO2);
            int mx = 1; for (int i = 0; i < 6; i++) if (c.bins[i] > mx) mx = c.bins[i];
            int bw = aw / 6, base = y + Ui.k(22) + gh - Ui.k(18);
            for (int i = 0; i < 6; i++) {
                int bhh = c.bins[i] * (gh - Ui.k(44)) / mx; g.setColor(i < 3 ? Ui.ROJO : 0x2E9E5B); if (i == 3) g.setColor(0x2E9E5B);
                g.fillRoundRect(m + i * bw + bw / 6, base - bhh, bw * 2 / 3, bhh + 1, Ui.k(4), Ui.k(4));
                Ui.centrado(g, "" + c.bins[i], m + i * bw + bw / 2, base - bhh - Ui.fS.getHeight() - 1, Ui.fS, Ui.TEXTO);
                Ui.centrado(g, i == 5 ? "6+" : "" + (i + 1), m + i * bw + bw / 2, base + Ui.k(3), Ui.fS, Ui.TEXTO2);
            }
            y += gh + Ui.k(34) + m;
            Ui.centrado(g, "Más baja " + Escala.coma(c.baja, d) + "  ·  más alta " + Escala.coma(c.alta, d), w / 2, y, Ui.fN, Ui.TEXTO2); y += Ui.fN.getHeight() + m;
            int rh = Ui.k(52), a = (y0 - y) / rh; if (a < 0) a = 0; int b = (y1 - y) / rh + 1; if (b > n) b = n;
            for (int i = a; i < b; i++) {
                Escala.Alumno al = (Escala.Alumno) c.alumnos.elementAt(i); int yy = y + i * rh;
                g.setColor((i & 1) == 0 ? 0xFFFFFF : 0xF3F6FB); g.fillRect(0, yy, w, rh);
                g.setFont(Ui.fN); g.setColor(Ui.TEXTO); g.drawString(Ui.recorta(al.nombre, Ui.fN, w - Ui.k(130)), Ui.k(16), yy + Ui.k(6), Graphics.TOP | Graphics.LEFT);
                g.setFont(Ui.fS); g.setColor(Ui.TEXTO2); g.drawString(Escala.pts(al.p) + " puntos", Ui.k(16), yy + Ui.k(8) + Ui.fN.getHeight(), Graphics.TOP | Graphics.LEFT);
                Ui.derecha(g, Escala.coma(al.n, d), w - Ui.k(16), yy + (rh - Ui.fL.getHeight()) / 2, Ui.fL, al.n >= e.napr - 1e-9 ? 0x1F7A43 : Ui.ROJO);
            }
            y += n * rh + m;
            if (c.ajustados > 0 || c.ignoradas > 0) { String s = (c.ajustados > 0 ? c.ajustados + " puntaje(s) fuera de rango se llevaron al límite. " : "") + (c.ignoradas > 0 ? c.ignoradas + " línea(s) sin puntaje no se incluyeron." : ""); y += Ui.lineas(g, Ui.envolver(s, Ui.fS, aw), m, y, Ui.fS, Ui.TEXTO2, 6) + m; }
            alto = y;
        }
        void dato(Graphics g, int x, int y, int w, String v, String et) { Ui.centrado(g, v, x + w / 2, y + Ui.k(14), Ui.fL, Ui.ACENTO); Ui.centrado(g, et, x + w / 2, y + Ui.k(14) + Ui.fL.getHeight() + Ui.k(6), Ui.fS, Ui.TEXTO2); }
    }

    // ============================================================ Acerca de
    final class Acerca extends Vista {
        Acerca() { titulo = "Acerca de"; }
        public void pintar(Graphics g, int w, int y0, int y1) {
            int m = Ui.k(18), aw = w - 2 * m, y = Ui.k(24);
            Ui.centrado(g, "Escala de Notas", w / 2, y, Ui.fL, Ui.ACENTO); y += Ui.fL.getHeight() + Ui.k(6);
            Ui.centrado(g, "Versión 2.0 para Symbian", w / 2, y, Ui.fS, Ui.TEXTO2); y += Ui.fS.getHeight() + Ui.k(16);
            String t = "Escala chilena por tramos (1,0 a 7,0) con cualquier exigencia y puntaje: desde la nota mínima hasta la de aprobación en el puntaje de exigencia, y de ahí hasta la nota máxima.\n\nCalcula qué nota es un puntaje, cuántos puntos pide una nota y las notas de un curso con promedio, mediana y aprobados.\n\nFunciona sin internet.\n\nDesarrollada por Javiera Navarrete.\nLicencia GPL-3.0-or-later.";
            y += Ui.lineas(g, Ui.envolver(t, Ui.fN, aw), m, y, Ui.fN, Ui.TEXTO, 40) + m; alto = y;
        }
    }
}
