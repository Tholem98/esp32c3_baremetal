#!/usr/bin/env bash
## flash.sh - Script simple para flashear la aplicación
## -----------------------------------------------
## Uso: ./flash.sh [PUERTO] [BAUD]
## Ej:  ./flash.sh /dev/ttyUSB0 460800
## Requiere que la imagen ya haya sido generada (make o build.sh)
set -e
BUILD_DIR=build
PORT=${1:-/dev/ttyACM0}      # Puerto serie por defecto
BAUD=${2:-460800}            # Baudrate por defecto

IMAGE="$BUILD_DIR/image"     # Nombre de archivo generado por elf2image

if [ ! -f $IMAGE ]; then
  echo "[ERROR] No se encontró $IMAGE. Ejecuta 'make' o './build.sh' primero." >&2
  exit 1
fi

echo "Flasheando aplicación en 0x10000 al puerto $PORT (baud $BAUD)"
esptool.py --chip esp32c3 --port $PORT --baud $BAUD write_flash 0x10000 $IMAGE

echo "Listo. (Recuerda: sin UART inicializada no verás texto en consola)"
echo "Para monitorear (cuando implementes UART):"
echo "  screen $PORT 115200   (o picocom/minicom)"
