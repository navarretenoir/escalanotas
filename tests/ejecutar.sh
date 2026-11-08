#!/usr/bin/env bash
# Pruebas de Escala de Notas: núcleo C (curso, exportación) y paridad núcleo C <-> interfaz web. Uso: tests/ejecutar.sh
set -euo pipefail
cd "$(dirname "$0")"
gcc -std=c89 -pedantic -Wall -Wextra -O1 -I../nucleo nucleo.c ../nucleo/escala.c -lm -o nucleo
./nucleo
gcc -std=c89 -pedantic -Wall -Wextra -O1 -I../nucleo paridad.c ../nucleo/escala.c -lm -o paridad
node paridad.js
node interfaz.js
rm -f paridad nucleo
