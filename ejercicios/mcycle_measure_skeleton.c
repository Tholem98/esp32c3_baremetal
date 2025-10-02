/* mcycle_measure_skeleton.c - Medición de periodo usando CSR mcycle
 * Objetivo: Capturar el número de ciclos entre toggles sin usar printf.
 * Pasos:
 *  1. Leer mcycle al inicio y al final de un ciclo de blink.
 *  2. Almacenar diferencias en un arreglo global.
 *  3. Inspeccionar con debugger (o volcar a UART cuando exista).
 */

#include <stdint.h>

static inline uint64_t mcycle_read(void){
    uint32_t lo, hi1, hi2;
    __asm__ volatile ("rdcycleh %0" : "=r"(hi1));
    __asm__ volatile ("rdcycle %0"  : "=r"(lo));
    __asm__ volatile ("rdcycleh %0" : "=r"(hi2));
    if(hi1!=hi2){
        __asm__ volatile ("rdcycle %0"  : "=r"(lo));
    }
    return ((uint64_t)hi2<<32)|lo;
}

/* TODO: Integrar en main o función separada para capturar N muestras. */
