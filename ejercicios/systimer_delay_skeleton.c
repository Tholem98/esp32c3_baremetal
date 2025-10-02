/* systimer_delay_skeleton.c - Esqueleto delay preciso con SYSTIMER
 * Objetivo: Implementar delay_us(uint32_t us) sin busy-wait de NOPs imprecisos.
 * Pasos:
 *  1. Inicializar SYSTIMER (si requiere configuración de fuente/reloj).
 *  2. Leer contador actual.
 *  3. Calcular valor objetivo (actual + us).
 *  4. Esperar (polling) hasta alcanzar objetivo.
 *  5. (Extensión) Cambiar a interrupciones.
 */

#include <stdint.h>

/* TODO: Definir base y offsets de SYSTIMER (comparadores / value low/high). */

void systimer_init(void) {
    /* TODO: Configuración inicial SYSTIMER (si aplica). */
}

void delay_us(uint32_t us) {
    /* TODO: Implementar espera activa basada en lectura de contador SYSTIMER. */
    (void)us; /* Quitar al implementar */
}
