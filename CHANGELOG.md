# Cambios / Changelog

## 1.1.0 (2026-11-08)
- Notas del curso: al pegar la lista se avisa cuántas líneas no tenían puntaje (antes se descartaban sin decir nada) y cuántos puntajes superaban el máximo y se llevaron al límite (posible error de tipeo). Lo mismo en las versiones de consola (DOS, OS/2).
- Exportación CSV: los nombres o títulos que empiezan con `=`, `+`, `-` o `@` se neutralizan para que Excel/Calc no los ejecuten como fórmula, y los textos con `;` o comillas van entre comillas. Aplica a la interfaz y al núcleo C.
- La exportación del núcleo C ahora informa error si el disco está lleno o el archivo no se puede cerrar (antes decía «guardado»).
- El puntaje máximo se limita a 400 también en la interfaz (como en el núcleo C); un valor absurdo ya no congela la página generando millones de filas.
- Contraste: los textos secundarios cumplen contraste AA en el tema claro y el oscuro; resultados y avisos se anuncian a lectores de pantalla; los botones de tema indican cuál está activo.
- Android: no se cierra la app si la vista web es terminada por el sistema ni ante datos de exportación dañados; los mensajes de guardado se entregan con `evaluateJavascript`; se cierran los archivos aunque falle la escritura.
- Pruebas: `tests/ejecutar.sh` suma pruebas del núcleo C (lectura del curso, CSV seguro, errores de escritura) y de la interfaz.
- Versiones unificadas en 1.1.0 (Android versionCode 2, Windows, Apple y paquetes).

## 1.0.1
- Se quita del repositorio el binario compilado de escritorio (`desktop/escala-notas`): ahora se genera con `make`. / The prebuilt desktop binary is no longer tracked; it is built with `make`.
- Se añade `packaging/` con recetas para Arch, Fedora/openSUSE y Debian. / Added `packaging/` recipes for Arch, Fedora/openSUSE and Debian.

## Primera versión pública / First public release
- Android 5.0+, Linux (RPM, DEB, AppImage, Flatpak, Snap), Windows, macOS/iOS. Fuentes y paquetes en las *releases*.
