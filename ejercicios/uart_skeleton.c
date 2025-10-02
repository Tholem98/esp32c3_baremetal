/* uart_skeleton.c - Esqueleto para implementación UART bare-metal (ESP32-C3)
 * Objetivo: completar inicialización básica de UART0 y función uart_putc.
 * Pasos sugeridos:
 *  1. Definir base de registros UART0.
 *  2. Configurar baudrate (divisores) para 115200.
 *  3. Implementar uart_putc bloqueante (esperar espacio en TXFIFO).
 *  4. (Opc) Implementar uart_puts iterando caracteres.
 *  5. Probar integrando llamado en main (NO incluido aún para no alterar ejemplo base).
 */

#include <stdint.h>

/* TODO: Definir direcciones base y offsets según TRM ESP32-C3 */
#define UART0_BASE 0x60000000UL /* EJEMPLO: reemplazar con dirección correcta */
/* TODO: Definir registros específicos: UART_FIFO_REG, UART_STATUS_REG, UART_CLKDIV_REG, etc. */

void uart_init(void) {
    /* TODO: Configurar divisor de reloj para 115200 bps
     * Formula típica: clk_uart / (divisor) = baud
     * Documentar en README o en la guía de ejercicios el cálculo realizado.
     */
}

void uart_putc(char c) {
    /* TODO: Esperar a que haya espacio en FIFO TX y escribir el byte */
    (void)c; /* Quitar al implementar */
}

void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}
