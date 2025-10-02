/* timer_interrupt_skeleton.c - Esqueleto para interrupción de timer
 * Objetivo: Configurar un timer (o SYSTIMER comparador) que dispare una interrupción periódica
 * para alternar un LED sin busy-wait.
 * Pasos sugeridos:
 *  1. Configurar mtvec (modo directo) apuntando a vector base.
 *  2. Habilitar fuente de interrupción específica en el controlador (pendiente investigar).
 *  3. Programar periodo.
 *  4. En handler: limpiar flag y togglear bit de GPIO.
 *  5. Medir jitter con analizador lógico.
 */

#include <stdint.h>

/* TODO: Definir registros del timer/SYSTIMER y del controlador de interrupciones. */

__attribute__((interrupt)) void irq_handler(void) {
    /* TODO: Identificar fuente, limpiar flag, togglear LED */
}

void timer_interrupt_init(void) {
    /* TODO: Configurar periodo y habilitar interrupción */
}
