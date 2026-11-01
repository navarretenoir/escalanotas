// Extrae las funciones de la interfaz web y compara con la salida del núcleo C.
const fs = require("fs"), cp = require("child_process");
const html = fs.readFileSync(__dirname + "/../ui/index.html", "utf8");
const js = html.slice(html.indexOf("<script>") + 8, html.lastIndexOf("</script>"));
const fn = n => { const i = js.indexOf("function " + n + "("); let d = 0, k = js.indexOf("{", i); for (let j = k; j < js.length; j++) { if (js[j] == "{") d++; else if (js[j] == "}" && --d == 0) return js.slice(i, j + 1); } };
eval(["num", "coma", "redondear", "notaDe", "puntosPara"].map(fn).join("\n") + "\nglobalThis.N = notaDe; globalThis.P = puntosPara;");
const c = cp.execFileSync(__dirname + "/paridad").toString().trim().split("\n");
let malas = 0;
for (const l of c) {
  const [cfg, rest] = l.split(":"), [pmax, ex, paso, modo] = cfg.split("/").map(Number);
  const e = { pmax, exig: ex / 100, paso, nmin: 1, napr: 4, nmax: 7, modo: ["aprox1", "trunc1", "aprox2"][modo] }; e.papr = pmax * e.exig;
  const [fil, pts] = rest.split("|"); const notasC = fil.split(",").filter(Boolean).map(Number);
  notasC.forEach((nc, i) => { const p = i * paso > pmax ? pmax : Math.round(i * paso * 100) / 100; if (Math.abs(N(p, e) - nc) > 0.0051) { if (malas++ < 5) console.log("DIFIERE", cfg, p, N(p, e), nc); } });
  const q = P(5.5, e); if (Math.abs(q - Number(pts)) > 0.0051) { if (malas++ < 5) console.log("DIFIERE puntos", cfg, q, pts); }
}
console.log(`${c.length} configuraciones comparadas · ${malas} diferencias`); process.exit(malas ? 1 : 0);
