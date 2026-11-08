// Pruebas de la interfaz web (ui/index.html) sin navegador: se ejecuta su script con un DOM mínimo falso.
const fs = require("fs"), vm = require("vm"), assert = require("assert");
const html = fs.readFileSync(__dirname + "/../ui/index.html", "utf8");
const js = html.slice(html.indexOf("<script>") + 8, html.lastIndexOf("</script>"));

function elemento(id, valor) {
  return { id, value: valor === undefined ? "" : valor, innerHTML: "", textContent: "", className: "", checked: false, style: {}, attrs: {},
    getAttribute(k) { return this.attrs[k]; }, setAttribute(k, v) { this.attrs[k] = v; }, getElementsByTagName() { return []; } };
}
function entorno() {
  const els = {}, guardados = [], valores = { pmax: "60", exig: "60", paso: "1", nmin: "1,0", napr: "4,0", nmax: "7,0", redondeo: "aprox1", "d-fecha": "2026-11-08" };
  const botones = ["pdf", "xlsx", "docx", "csv", "png", "copiar"].map(t => Object.assign(elemento("exp-" + t), { attrs: { "data-exp": t } }));
  const document = {
    getElementById: id => els[id] || (els[id] = elemento(id, valores[id])),
    querySelectorAll: sel => (sel === "[data-exp]" ? botones : []),
    documentElement: { setAttribute() {}, removeAttribute() {} },
    createElement: () => elemento("x"), body: { appendChild() {}, removeChild() {} },
  };
  const window = { EscalaNativa: { guardar: (n, m, b64) => guardados.push({ n, m, texto: Buffer.from(b64, "base64").toString("utf8") }) } };
  const ctx = { window, document, localStorage: { getItem: () => null, setItem() {} }, Uint8Array, unescape, encodeURIComponent, btoa: s => Buffer.from(s, "binary").toString("base64"),
    atob: s => Buffer.from(s, "base64").toString("binary"), Date, Math, JSON, String, isNaN, parseFloat, Number, Array, setTimeout, clearTimeout };
  vm.createContext(ctx); vm.runInContext(js, ctx);
  return { els, guardados, botones, poner(id, v) { document.getElementById(id).value = v; const e = els[id]; if (e.oninput) e.oninput(); } };
}

// 1. el curso avisa de líneas ignoradas y de puntajes ajustados
let t = entorno();
t.poner("lista", "Ana Pérez  48\nBenjamín Soto 31,5\nSolo nombre\nCarla 480");
let r = t.els["curso-res"].innerHTML;
assert.ok(/1 puntaje superaba el máximo de 60/.test(r), "aviso de puntaje ajustado: " + r.slice(0, 200));
assert.ok(/1 línea sin puntaje no se incluyó/.test(r), "aviso de línea ignorada");
assert.ok(/3 de 3 aprobados|aprobados/.test(r));

// 2. sin ninguna línea válida se explica el formato en vez de quedar en blanco
t = entorno(); t.poner("lista", "solo texto");
assert.ok(/Ninguna línea tiene un puntaje válido/.test(t.els["curso-res"].innerHTML));

// 3. un puntaje máximo absurdo no genera millones de filas
t = entorno(); t.poner("pmax", "6000000");
assert.ok(/Revisa los datos/.test(t.els.resumen.innerHTML), "pmax > 400 se rechaza");
assert.strictEqual(t.els.tabla.innerHTML, "");

// 4. el CSV no deja pasar fórmulas por los nombres
t = entorno(); t.poner("lista", "=HYPERLINK(\"http://x\")  50\n+cmd  40\nAna; Pérez  48\nNormal  30");
t.botones.find(b => b.attrs["data-exp"] === "csv").onclick();
assert.strictEqual(t.guardados.length, 1, "debe entregar un CSV");
const csv = t.guardados[0].texto;
assert.ok(csv.includes("\"'=HYPERLINK(\"\"http://x\"\")\";50"), "fórmula neutralizada:\n" + csv.slice(-300));
assert.ok(csv.includes("\"'+cmd\";40"), "+cmd neutralizado");
assert.ok(csv.includes("\"Ana; Pérez\";48"), "separador dentro del nombre va entre comillas");
assert.ok(csv.includes("\r\nNormal;30;"), "nombre normal intacto");

// 5. accesibilidad básica del documento
assert.ok(!/user-scalable\s*=\s*no|maximum-scale/i.test(html), "no debe bloquear el zoom");
assert.ok(/id="q-pts-r"[^>]*aria-live/.test(html));
console.log("interfaz: pruebas OK");
