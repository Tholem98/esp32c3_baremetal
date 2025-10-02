# ESP32-C3 Bare-Metal (Sin ESP-IDF / Sin FreeRTOS)

> Sección nueva: Punto de partida para alumnos

## Inicio Rápido para Alumnos

1. Verifica toolchain: `riscv32-esp-elf-gcc --version` y `esptool.py --version`.
2. Compila: `make`
3. Flashea (ajusta puerto si es necesario): `make flash` o `./flash.sh /dev/ttyUSB0 460800`
4. Conecta un LED (GPIO4 -> Resistencia 220Ω -> Anodo LED -> Cátodo a GND).
5. Abre la guía de ejercicios: ver archivo `GUIA_DE_EJERCICIOS.md` en la raíz.
6. Explora skeletons en carpeta `ejercicios/`.

Orden sugerido de aprendizaje:
GPIO básico → medición con mcycle → UART → timers → interrupciones → refactor modular.

En caso de dudas sobre memoria, revisar secciones 4 y 6 de este README.


Proyecto pedagógico ultra minimal para **Sistemas Digitales II**. Objetivo: mostrar el flujo completo de arranque, memoria y control de GPIO sin usar el framework ESP-IDF ni FreeRTOS.

---

## 1. ¿Qué hace este proyecto?

Parpadea un pin GPIO (ahora configurado en el código como GPIO4; originalmente ejemplo usaba GPIO2) usando:

- Startup propio (`startup.S`)
- Linker script manual (`linker.ld`)
- Código C mínimo (`main.c`)
- Acceso directo a registros (TRM ESP32-C3)

No hay scheduler, no hay drivers, no hay librerías estándar (no `printf`, no `malloc`).

---

## 2. Estructura de Archivos

```text
esp32c3_raw_bare/
├── linker.ld          # Script de link: define memoria, secciones y símbolos
├── Makefile           # Compilación con 'make'
├── build.sh           # Script alternativo de build paso a paso
├── flash.sh           # Flasheo rápido de la imagen generada
├── src/
│   ├── startup.S      # Código de arranque (reset vector)
│   └── main.c         # Lógica de blink
└── include/           # (vacío por ahora, para futuras cabeceras propias)
```

---

## 3. Flujo de Arranque (Resumen)

1. Chip resetea → Boot ROM (enmascarada) inicializa lo básico y carga la imagen de flash (si existe bootloader, lo ejecuta; aquí asumimos bootloader estándar ya flasheado anteriormente por ESP-IDF o fábrica).
2. Se mapea el binario de la app a partir de 0x42000000 (flash mapeada XIP).
3. Nuestro `_start` (en `startup.S`):
   - Configura el stack pointer (`sp`).
   - Limpia `.bss` (variables globales no inicializadas → cero).
   - Copia `.data` desde flash (después de `.text`) a RAM.
   - Llama a `main`.
   - Si `main` retorna, entra a un bucle infinito.

---

## 4. Linker Script Clave (`linker.ld`) y Mapa de Memoria

Se definen dos regiones de memoria principales (simplificadas para docencia):

```ld
MEMORY {
  IROM (rx)  : ORIGIN = 0x42000000, LENGTH = 2M
  DRAM (rwx) : ORIGIN = 0x3FC80000, LENGTH = 400K
}
```

Se crean símbolos pedagógicos:

- `_stext` / `_etext`: delimitan código y rodata.
- `_sdata` / `_edata`: datos inicializados (RAM).
- `_sbss` / `_ebss`: datos a cero.
- `_stack_top`: tope de la pila.

`startup.S` usa estos símbolos para inicializar memoria.

### 4.1 Mapa de Memoria (Resumen Pedagógico)

| Región | Dirección Inicio | Tamaño aprox | Contenido | Notas |
|--------|------------------|--------------|-----------|-------|
| Boot ROM (enmascarada) | 0x0000_0000 | (fija) | Código ROM Espressif | No modificable; ejecuta bootloader interno / carga app |
| Flash SPI externa | (física) | Según módulo | Contiene bootloader, particiones, app, datos | Mapeada parcialmente vía caché XIP |
| Flash mapeada XIP | 0x4200_0000 | Ventana de 2 MB usada aquí | Código (.text) + .rodata ejecutables | Nuestra app se ejecuta directamente desde aquí |
| DRAM principal | 0x3FC8_0000 | ~400 KB (simplificado) | .data, .bss, stack, (heap) | Acceso de lectura/escritura rápido |
| Registros Periféricos (ej. GPIO) | 0x6000_0000+ | Espaciado por bloques | Control hardware | Acceso por direcciones fijas |

Direcciones clave empleadas:

- Base GPIO: 0x6000_4000 (bloque de control de pines generales)
- Ventana flash XIP usada: 0x4200_0000 (donde el enlazador coloca .text)
- Inicio DRAM: 0x3FC8_0000 (donde ubicamos datos y stack)

### 4.2 Flujo de Colocación

1. El enlazador coloca `.text` y `.rodata` en la región IROM (FLASH mapeada).
2. Ubica `.data` en DRAM pero especifica su Load Memory Address (LMA) justo a continuación de `.text` en FLASH.
3. `.bss` se reserva en DRAM sin ocupar espacio en el binario (NOLOAD) y se limpia a cero.
4. `_stack_top` marca el final de la región DRAM como tope de la pila (simplificación).

### 4.3 OFFSETS Bootloader (Contexto)

El ejemplo asume que ya existe un bootloader estándar (cargado previamente) que:

- Vive en flash en offset 0x0.
- Carga nuestra imagen de aplicación ubicada comúnmente a partir de 0x10000 dentro de la flash física.
- Nuestra imagen se mapea en la ventana 0x4200_0000 para ejecución.

Para un escenario totalmente bare-metal sin bootloader preexistente habría que:

1. Crear un boot mínimo en 0x0 que configure caché/mapeo.
2. O reubicar el código para arrancar directamente sin las facilidades del bootloader (más complejo y fuera del alcance inmediato del curso inicial).

---

## 5. Startup (`startup.S`)

Pseudocódigo:

```text
sp = _stack_top
memset(.bss, 0)
copy(flash: después de .text → RAM .data)
call main()
loop para siempre
```

No configuramos interrupciones todavía; se puede extender agregando vector table y habilitando mtvec.

---

## 6. Blink (`main.c`) y Registros GPIO

El parpadeo se implementa alternando un bit en los registros de salida. En la versión actual se usa `GPIO4` (define `LED_GPIO` en el código). Antes se mostró `GPIO2` solo como ejemplo.

### 6.1 Base de GPIO

- Base de registros GPIO empleada: `0x6000_4000`

### 6.2 Registros usados (offset respecto a base)

| Registro | Offset | Acción al escribir | Propósito |
|----------|--------|--------------------|-----------|
| GPIO_OUT_W1TS_REG | 0x0008 | Bits en 1 -> pone esos pines a nivel ALTO | Encender LED (set) |
| GPIO_OUT_W1TC_REG | 0x000C | Bits en 1 -> pone esos pines a nivel BAJO | Apagar LED (clear) |
| GPIO_ENABLE_W1TS_REG | 0x0024 | Bits en 1 -> habilita dirección de salida | Configurar pin como salida |

No se usa lectura: las operaciones W1TS/W1TC se diseñan para no afectar otros pines (evitan read-modify-write).

### 6.3 Secuencia del bucle principal

1. Habilitar el pin como salida: escribir `LED_MASK` en `GPIO_ENABLE_W1TS_REG`.
2. Escribir `LED_MASK` en `GPIO_OUT_W1TS_REG` → LED ON.
3. Delay (busy-wait) con NOPs.
4. Escribir `LED_MASK` en `GPIO_OUT_W1TC_REG` → LED OFF.
5. Repetir.

### 6.4 Observaciones de Tiempo

El delay basado en NOPs no es exacto y depende de la frecuencia de CPU (160 MHz típica). Para un control más preciso se propondrá uso de SYSTIMER o un timer de hardware en extensiones futuras.

---

## 7. Compilación

### Opción A (Makefile)

```bash
make
```

Genera en `build/`:

- `app.elf`
- `app.bin`
- `app.map`
- `app.dis` (desensamblado)
- `image` (salida de `esptool.py elf2image` con el prefijo usado)

### Opción B (Script paso a paso)

```bash
./build.sh
```

> Requiere tener `riscv32-esp-elf-gcc` en el PATH (se activa con `source $HOME/esp/esp-idf/export.sh` o agregando manualmente el path de toolchain).

---

## 8. Flasheo

Flashear solo la app (asumiendo bootloader ya existe):

```bash
./flash.sh /dev/ttyACM0 460800
```

O manual:

```bash
esptool.py --chip esp32c3 write_flash 0x10000 build/image
```

Si prefieres un nombre distinto o asegurar formato legacy, puedes usar:

```bash
esptool.py --chip esp32c3 elf2image build/app.elf --version 2 --output build/app_v2
esptool.py --chip esp32c3 write_flash 0x10000 build/app_v2
```

Importante: Esto asume que ya existe en flash (offset 0x0) un bootloader y tabla de particiones estándar. Si la placa nunca fue flasheada con ESP-IDF, primero crea y flashea un proyecto trivial (hello_world) con `idf.py flash` para instalar bootloader/partitions. Alternativa más avanzada (no cubierta todavía): crear un bootloader bare-metal y mapear tu código a 0x0.

Si no tienes bootloader, puedes primero flashear uno desde un proyecto ESP-IDF y luego usar este flujo.

---

## 9. Monitor Serie

Este proyecto no incluye UART init ni `printf`.

Si deseas salida serial bare-metal:

1. Implementar init de UART0 (configurar baud, pins).  
2. Escribir en el FIFO TX registrando bits de estado.  
3. Reemplazar `delay()` por toggling + envío de caracteres.  

(Se deja como ejercicio avanzado.)

---

## 10. Extensiones Sugeridas para Estudiantes

| Tema | Ejercicio | Dificultad |
|------|-----------|------------|
| UART | Implementar `uart_putc` y enviar "Hello" | ★★ |
| Timer | Usar SYSTIMER para delays precisos | ★★ |
| Interrupciones | Crear vector mtvec y handler dummy | ★★★ |
| Assembly | Reescribir delay en ensamblador optimizado | ★ |
| GPIO input | Leer botón y cambiar patrón de blink | ★ |
| PWM básico | Software PWM con busy loop | ★★ |
| Medición cycles | Leer CSR `mcycle` y calcular periodo | ★★ |

---

## 11. Depuración (Opcional JTAG / USB-JTAG)

Con OpenOCD (si instalado):

```bash
openocd -f board/esp32c3-builtin.cfg
```

Luego:

```bash
riscv32-esp-elf-gdb build/app.elf
(gdb) target remote :3333
(gdb) monitor reset halt
(gdb) load
(gdb) continue
```

---

## 12. Validación Didáctica

Checklist para alumnos:

- ¿Pueden explicar la diferencia entre LMA y VMA en `.data`?
- ¿Identifican en `app.map` dónde quedó cada sección?
- ¿Pueden mostrar en `app.dis` la instrucción que alterna el bit del GPIO?
- ¿Pueden estimar el retardo real midiendo con analizador lógico?

---

## 13. Limitaciones

- Sin WiFi/BLE (stacks requieren entorno del SDK).
- Sin seguridad/crypto de alto nivel.
- Sin memoria dinámica (se puede agregar un heap simple si se requiere).

---

## 14. Próximos Pasos Propuestos

1. Añadir `uart.c` para tener un `putc` y un `puts` simples.
2. Implementar vector de interrupciones y habilitar una interrupción de timer.
3. Crear un mini "driver" de delay basado en SYSTIMER en lugar de busy-wait.
4. Medir jitter del parpadeo vs versión con FreeRTOS.

---

## 15. Referencias

- ESP32-C3 Technical Reference Manual (TRM)
- RISC-V Privileged Spec (para comprender CSR y mtvec)
- Espressif esptool.py docs

---

## 16. Licencia y Uso

Material educativo. Puedes adaptar libremente para tu curso.

---

¡Listo para enseñar bare-metal real! Extiende paso a paso sin saltar directo a complejidad.
