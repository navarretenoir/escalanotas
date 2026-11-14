#!/usr/bin/env bash
# Escala de Notas para Symbian (Java ME). Requiere el kit ~/.local/share/kotone-herramientas/j2me (ecj + ProGuard + CLDC/MIDP).
exec ~/.local/share/kotone-herramientas/j2me/construir-midlet.sh "$(dirname "$(readlink -f "$0")")" "Escala de Notas" cl.kotone.escala.EscalaMidlet 2.0.0 ninguno ~/Proyectos/SYMBIAN_KIT/src
