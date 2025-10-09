/*
 * main.c - Ejemplo mínimo bare-metal (blink) para ESP32-C3
 * --------------------------------------------------------
 * OBJETIVO: Mostrar cómo controlar un GPIO escribiendo registros directamente,
 *           sin ESP-IDF ni FreeRTOS, y sin librerías estándar (no printf, no malloc).
 *
 * FLUJO:
 *  - El startup inicializa memoria y llama a main.
 *  - Configuramos el GPIO como salida.
 *  - Entramos en un bucle infinito alternando el nivel lógico del pin.
 *
 * REGISTROS GPIO USADOS (base 0x6000_4000 según TRM ESP32-C3):
 *  - GPIO_OUT_W1TS_REG (offset 0x0008): Escribir bits en 1 pone esos pines en nivel alto.
 *  - GPIO_OUT_W1TC_REG (offset 0x000C): Escribir bits en 1 limpia (pone a 0) esos pines.
 *  - GPIO_ENABLE_W1TS_REG (offset 0x0024): Escribir bits en 1 habilita el modo salida del pin.
 *
 * NOTA: Operamos con registros *write-one-to-set* / *write-one-to-clear* para no alterar otros pines.
 */
#include <stdint.h>

/* Dirección base de los registros de GPIO (TRM ESP32-C3). */
#define GPIO_BASE              0x60004000UL
#define GPIO_OUT_REG           (*(volatile uint32_t*)(GPIO_BASE + 0x0004))
#define GPIO_OUT_W1TS_REG      (*(volatile uint32_t*)(GPIO_BASE + 0x0008))
#define GPIO_OUT_W1TC_REG      (*(volatile uint32_t*)(GPIO_BASE + 0x000C))
#define GPIO_ENABLE_W1TS_REG   (*(volatile uint32_t*)(GPIO_BASE + 0x0024))

/* Selección del pin del LED (cambiado de GPIO2 a GPIO4 para evitar pin de strapping). */
#define LED_GPIO 3
#define LED_MASK (1U << LED_GPIO)

/* delay(): Espera ocupada (busy-wait). Cada iteración ejecuta un NOP.
 * - No es precisa (depende de la frecuencia de CPU y optimizaciones).
 * - Se reemplazará en ejercicios futuros por un timer hardware o SYSTIMER.
 */
static void delay(volatile uint32_t cycles) {
    while (cycles--) {
        __asm__ volatile ("nop");
    }
}

int main(void) {
    /* Configurar LED_GPIO como salida (pone a 1 el bit correspondiente en el registro ENABLE_W1TS). */
    GPIO_ENABLE_W1TS_REG = LED_MASK;

    while (1) {
        /* Poner el LED en alto escribiendo 1 en su bit (GPIO_OUT_W1TS_REG). */
        GPIO_OUT_W1TS_REG = LED_MASK;
        delay(400000); /* ~visible ON time */

    /* Poner el LED en bajo limpiando el bit (GPIO_OUT_W1TC_REG). */
        GPIO_OUT_W1TC_REG = LED_MASK;
        delay(400000); /* ~visible OFF time */
    }
}
