package cl.kotone.escala;

import java.util.Vector;

/** Núcleo de la escala chilena por tramos (nota mínima → aprobación en el puntaje de exigencia → nota máxima). Misma fórmula que la versión web. */
final class Escala {
    static final int APROX1 = 0, TRUNC1 = 1, APROX2 = 2;
    double pmax = 60, exig = 0.6, paso = 1, nmin = 1, napr = 4, nmax = 7, papr;
    int modo = APROX1;

    boolean ok() { return pmax > 0 && pmax <= 400 && exig > 0 && exig < 1 && nmin < napr && napr < nmax && paso > 0; }
    void preparar() { papr = pmax * exig; }
    int dec() { return modo == APROX2 ? 2 : 1; }

    double redondear(double n) {
        if (modo == TRUNC1) return Math.floor(n * 10 + 1e-9) / 10;
        if (modo == APROX2) return Math.floor(n * 100 + 1e-9 + 0.5) / 100;
        return Math.floor(n * 10 + 1e-9 + 0.5) / 10;
    }
    double nota(double p) {
        double n = p < papr ? nmin + (napr - nmin) * p / papr : napr + (nmax - napr) * (p - papr) / (pmax - papr);
        return Math.min(nmax, Math.max(nmin, redondear(n)));
    }
    /** mínimo puntaje (en múltiplos del paso) que alcanza la nota; -1 si no se alcanza */
    double puntosPara(double nota) {
        if (nota <= nmin) return 0;
        if (nota > nmax) return -1;
        int n = (int) Math.floor(pmax / paso + 1e-9);
        for (int i = 0; i <= n; i++) { double p = i * paso; if (nota(p) >= nota - 1e-9) return Math.floor(p * 100 + 0.5) / 100; }
        return -1;
    }
    int filas() { int n = (int) Math.floor(pmax / paso + 1e-9); return n + 1 + (n * paso < pmax - 1e-9 ? 1 : 0); }
    double puntoFila(int i) { double p = i * paso; return p > pmax ? pmax : Math.floor(p * 100 + 0.5) / 100; }

    // ---- formato
    static String coma(double x, int dec) {
        boolean neg = x < 0; if (neg) x = -x;
        long f = 1; for (int i = 0; i < dec; i++) f *= 10;
        long v = (long) Math.floor(x * f + 0.5);
        String ent = String.valueOf(v / f), r = ent;
        if (dec > 0) { String fr = String.valueOf(v % f); while (fr.length() < dec) fr = "0" + fr; r = ent + "," + fr; }
        return neg ? "-" + r : r;
    }
    static String pts(double p) {
        double q = Math.floor(p * 100 + 0.5) / 100;
        if (q == Math.floor(q)) return String.valueOf((long) q);
        String s = coma(q, 2); return s.endsWith("0") ? s.substring(0, s.length() - 1) : s;
    }
    /** acepta coma o punto; NaN si no es número */
    static double num(String s) {
        if (s == null) return Double.NaN;
        StringBuffer b = new StringBuffer();
        for (int i = 0; i < s.length(); i++) { char c = s.charAt(i); if (c == ',') c = '.'; if (c > ' ') b.append(c); }
        if (b.length() == 0) return Double.NaN;
        try { return Double.parseDouble(b.toString()); } catch (Exception e) { return Double.NaN; }
    }

    // ---- curso
    static final class Alumno { String nombre; double p, n; }
    static final class Curso {
        Vector alumnos = new Vector(); int ignoradas, ajustados, aprobados; double promedio, mediana, baja, alta; int[] bins = new int[6];
    }

    /** «Ana Pérez  48» por línea (separadores: espacio, tab, ; , |). Una línea sólo con número = «Estudiante N». */
    Curso curso(String texto) {
        Curso c = new Curso();
        int ini = 0, L = texto.length();
        while (ini <= L) {
            int fin = texto.indexOf('\n', ini); if (fin < 0) fin = L;
            String t = texto.substring(ini, fin).trim(); ini = fin + 1;
            if (t.length() == 0) { if (fin >= L) break; continue; }
            int j = t.length();
            while (j > 0 && (Character.isDigit(t.charAt(j - 1)) || t.charAt(j - 1) == '.' || t.charAt(j - 1) == ',')) j--;
            String numS = t.substring(j), nombre = t.substring(0, j);
            if (numS.length() == 0 || !Character.isDigit(numS.charAt(0)) && numS.length() < 2 || Double.isNaN(num(numS))) { c.ignoradas++; if (fin >= L) break; continue; }
            if (j > 0 && nombre.length() > 0 && " \t;,|".indexOf(nombre.charAt(nombre.length() - 1)) < 0) { c.ignoradas++; if (fin >= L) break; continue; }
            while (nombre.length() > 0 && " \t;,|".indexOf(nombre.charAt(nombre.length() - 1)) >= 0) nombre = nombre.substring(0, nombre.length() - 1);
            Alumno a = new Alumno();
            a.nombre = nombre.length() == 0 ? "Estudiante " + (c.alumnos.size() + 1) : nombre;
            double p = num(numS); if (p < 0 || p > pmax) { c.ajustados++; p = Math.min(Math.max(p, 0), pmax); }
            a.p = p; a.n = nota(p); c.alumnos.addElement(a);
            if (fin >= L) break;
        }
        int n = c.alumnos.size(); if (n == 0) return c;
        double[] ns = new double[n]; double suma = 0;
        for (int i = 0; i < n; i++) {
            Alumno a = (Alumno) c.alumnos.elementAt(i); ns[i] = a.n; suma += a.n;
            if (a.n >= napr) c.aprobados++;
            c.bins[Math.min(5, Math.max(0, (int) Math.floor(a.n) - 1))]++;
        }
        for (int i = 1; i < n; i++) { double x = ns[i]; int k = i - 1; while (k >= 0 && ns[k] > x) { ns[k + 1] = ns[k]; k--; } ns[k + 1] = x; }
        c.promedio = suma / n; c.mediana = n % 2 == 1 ? ns[(n - 1) / 2] : (ns[n / 2 - 1] + ns[n / 2]) / 2; c.baja = ns[0]; c.alta = ns[n - 1];
        return c;
    }
}
