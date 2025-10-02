#!/usr/bin/env bash
## build.sh - Secuencia paso a paso (didáctica) de construcción
## -----------------------------------------------------------
## Muestra explícitamente cada comando sin depender de Makefile.
## Útil para que los alumnos vean y modifiquen flags manualmente.
set -e

TARGET=app
BUILD_DIR=build

mkdir -p $BUILD_DIR

echo "[1/4] Compilando fuentes (startup + main)"  # Genera objetos .o
riscv32-esp-elf-gcc -Os -march=rv32imc -mabi=ilp32 -ffreestanding -nostdlib -Wall -Wextra \
    -Iinclude -c src/startup.S -o $BUILD_DIR/startup.o
riscv32-esp-elf-gcc -Os -march=rv32imc -mabi=ilp32 -ffreestanding -nostdlib -Wall -Wextra \
    -Iinclude -c src/main.c -o $BUILD_DIR/main.o

echo "[2/4] Enlazando objetos -> ELF final"       # Aplica linker.ld
riscv32-esp-elf-gcc -T linker.ld -nostdlib -nostartfiles \
    $BUILD_DIR/startup.o $BUILD_DIR/main.o -o $BUILD_DIR/$TARGET.elf -Wl,-Map=$BUILD_DIR/$TARGET.map

echo "[3/4] Generando binario plano e imagen para flasheo"  # objcopy + elf2image
riscv32-esp-elf-objcopy -O binary $BUILD_DIR/$TARGET.elf $BUILD_DIR/$TARGET.bin
esptool.py --chip esp32c3 elf2image $BUILD_DIR/$TARGET.elf --output $BUILD_DIR/image

echo "[4/4] Resumen de tamaños de secciones"    # text/data/bss
riscv32-esp-elf-size $BUILD_DIR/$TARGET.elf

echo "Listo. Archivos en $BUILD_DIR:"
ls -1 $BUILD_DIR

echo "Para flashear (solo app, requiere bootloader existente):"
echo "  esptool.py --chip esp32c3 write_flash 0x10000 $BUILD_DIR/image"  # Imagen generada
echo "Nota: Debe existir bootloader + partition table en 0x0 (flashear un proyecto ESP-IDF si no)."
