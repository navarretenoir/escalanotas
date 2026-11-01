# Escala de Notas — GTK 4 + WebKitGTK 6. Desarrollado por Javiera Navarrete.
PKGS := gtk4 webkitgtk-6.0 json-glib-1.0
UI_DIR ?= /usr/share/escala-notas/ui
CFLAGS := -O2 -Wall -Wextra -std=c11 $(shell pkg-config --cflags $(PKGS)) -DUI_DIR='"$(UI_DIR)"' $(EXTRA_CFLAGS)
LIBS := $(shell pkg-config --libs $(PKGS))
escala-notas: escala-notas.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
clean:
	rm -f escala-notas
