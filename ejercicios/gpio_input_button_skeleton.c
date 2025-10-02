/* gpio_input_button_skeleton.c - Lectura de botón y cambio de frecuencia de blink
 * Objetivo: Detectar pulsaciones y variar delay del LED.
 * Pasos sugeridos:
 *  1. Configurar pin como entrada (registros ENABLE_W1TC para asegurarse, y dirección input por default).
 *  2. (Opcional) Configurar pull-up/pull-down externo (hardware) o documentar.
 *  3. Leer estado y usar lógica para seleccionar un delay (lento/rápido).
 *  4. Implementar anti-rebote simple (contador o pequeña espera cuando cambia estado).
 */

#include <stdint.h>

/* TODO: Definir base GPIO si se reutiliza aquí, o incluir header común si se factoriza. */

int button_read(void) {
    /* TODO: Leer registro de entrada (investigar en TRM el registro de estado) */
    return 0; /* Placeholder */
}
