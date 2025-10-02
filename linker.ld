/*
 * linker.ld - Script de enlace mínimo para ESP32-C3 (versión didáctica)
 * ---------------------------------------------------------------
 * OBJETIVO PEDAGÓGICO:
 *  - Mostrar explícitamente cómo el enlazador coloca secciones en memoria.
 *  - Diferenciar entre FLASH mapeada (XIP) y RAM para datos.
 *  - Exponer símbolos que el startup usa para copiar/limpiar secciones.
 *
 * NOTAS:
 *  - El ESP32-C3 ejecuta código en flash mapeada (XIP) a través de la región 0x4200_0000.
 *  - La memoria DRAM principal accesible a la CPU comienza en 0x3FC8_0000.
 *  - LENGTH aquí es una simplificación (no todo el espacio está siempre disponible, pero para un ejemplo pequeño basta).
 *  - No incluimos particiones, bootloader ni secciones avanzadas (vector interrupciones separado, etc.).
 *
 * SÍMBOLOS EXPUESTOS:
 *  _stext/_etext : Delimitan el bloque de código (.text + .rodata) que queda en FLASH.
 *  _sdata/_edata : Datos inicializados que se COPIAN desde FLASH a RAM al arranque.
 *  _sbss/_ebss   : Zona BSS que se pone a cero en startup.
 *  _stack_top    : Dirección usada para inicializar el stack pointer (SP).
 *  _sheap/_eheap : Marcadores pedagógicos de un posible heap (no usado aún).
 */

ENTRY(_start)

MEMORY
{
  /* Región de FLASH mapeada a la ventana ejecutable (XIP). Código y rodata permanecen aquí. */
  IROM (rx)  : ORIGIN = 0x42000000, LENGTH = 2M
  /* Región de DRAM: almacenará .data copiada, .bss y la pila (stack). */
  DRAM (rwx) : ORIGIN = 0x3FC80000, LENGTH = 400K
}

/* Externally visible symbols */
PROVIDE(_stack_top = ORIGIN(DRAM) + LENGTH(DRAM));

SECTIONS
{
  /* Sección .text: código ejecutable + .rodata permanecen en FLASH (IROM). */
  .text : {
    _stext = .;              /* Marca inicio del bloque de código/rodata */
    *(.init)                 /* Código de inicio (startup) */
    *(.text*)                /* Código C/asm */
    *(.rodata*)              /* Datos de solo lectura (const) */
    . = ALIGN(4);
    _etext = .;              /* Fin de la porción que permanece en FLASH */
  } > IROM

  /* Sección .data: datos inicializados que deben terminar en RAM.
   * AT(...) indica su LOAD ADDRESS en FLASH (justo tras .text) para ser copiados. */
  .data : AT (LOADADDR(.text) + SIZEOF(.text)) {
    . = ALIGN(4);
    _sdata = .;              /* Inicio de .data en RAM */
    *(.data*)
    . = ALIGN(4);
    _edata = .;              /* Fin de .data */
  } > DRAM

  /* Sección .bss: variables globales no inicializadas -> se llenan con cero en runtime. */
  .bss (NOLOAD) : {
    . = ALIGN(8);
    _sbss = .;               /* Inicio de .bss */
    *(.bss*)
    *(COMMON)
    . = ALIGN(8);
    _ebss = .;               /* Fin de .bss */
  } > DRAM

  /* Marcadores de heap simple (no implementado). Dejamos 0x2000 (~8KB) para stack. */
  _sheap = _ebss;
  _eheap = _stack_top - 0x2000; /* Reservar espacio para la pila (simplificación). */
}
