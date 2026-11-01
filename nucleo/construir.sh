#!/usr/bin/env bash
# Compila Escala de Notas para sistemas antiguos con Open Watcom (instalado en /opt/watcom). Desarrollada por Javiera Navarrete.
#  DOS (16 bits) · OS/2 Warp (32 bits, consola) · Windows 3.x (16 bits) · Windows NT 3.5/3.51/4.0, 9x/Me y 2000–8 (32 bits)
set -euo pipefail
source /opt/watcom/entorno.sh
cd "$(dirname "$(readlink -f "$0")")"
O=$(pwd)/../dist/antiguos; mkdir -p "$O" build; rm -f build/*
cp escala.c escala.h escala_con.c escala_win.c build/ && cd build
export INCLUDE=$WATCOM/h
wcl -q -bt=dos -ml -os -fe=ESCALA.EXE escala_con.c escala.c                         && mv ESCALA.EXE "$O/ESCALA-DOS.EXE"
INCLUDE=$WATCOM/h/os2:$WATCOM/h wcl386 -q -bt=os2 -l=os2v2 -os -fe=escala-os2.exe escala_con.c escala.c              && mv escala-os2.exe "$O/ESCALA-OS2.EXE"
INCLUDE=$WATCOM/h/win:$WATCOM/h wcl -q -bt=windows -ml -l=windows -os -fe=ESCALA16.EXE escala_win.c escala.c         && mv ESCALA16.EXE "$O/ESCALA-WIN3X.EXE"
INCLUDE=$WATCOM/h/nt:$WATCOM/h wcc386 -q -bt=nt -os escala_win.c && INCLUDE=$WATCOM/h wcc386 -q -bt=nt -os escala.c \
  && wlink system nt_win option version=3.10,quiet name ESCALA32.EXE file escala_win.o,escala.o && mv ESCALA32.EXE "$O/ESCALA-WIN32.EXE"
(cd "$O" && sha256sum *.EXE > SHA256SUMS.txt; ls -la)
