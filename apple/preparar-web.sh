#!/usr/bin/env bash
# Copia la interfaz compartida (../ui) a Recursos/web para macOS/iOS. Desarrollada por Javiera Navarrete.
set -euo pipefail
A=$(cd "$(dirname "$0")" && pwd); W="$A/Recursos/web"
rm -rf "$W"; mkdir -p "$W"; cp -r "$A/../ui/." "$W/"; rm -f "$W"/*.bak*
echo "✔ Recursos/web listo"
