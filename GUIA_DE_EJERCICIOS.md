# Guía de Ejercicios - ESP32-C3 Bare-Metal

Esta guía propone una progresión incremental para que los alumnos comprendan el entorno bare-metal, el hardware del ESP32-C3 y las implicaciones de cada capa de abstracción.

## Reglas Generales

- Trabajar SIEMPRE en commits pequeños y descriptivos.
- Mantener cada ejercicio en una rama distinta (ej: `ej1-gpio`, `ej2-delay-mcycle`).
- Documentar en un README propio (por ejercicio) lo aprendido y problemas encontrados.
- No introducir librerías externas sin justificación.

## Tabla de Progresión

| Nº | Tema Central | Objetivo Principal | Dificultad | Entregable |
|----|--------------|-------------------|------------|-----------|
| 1 | GPIO Básico | Cambiar pin del LED y ajustar frecuencia visible | ★ | Código + explicación |
| 2 | Ciclos (mcycle) | Medir periodo real del blink vs cálculo teórico | ★★ | Cálculo + tabla |
| 3 | Delay preciso | Implementar delay calibrado con mcycle | ★★ | Función `delay_cycles()` |
| 4 | UART TX | Enviar texto "Hola" y secuencia de estado | ★★ | `uart_putc`, `uart_init` |
| 5 | Botón entrada | Cambiar frecuencia según botón (polling) | ★★ | Código + diagrama estados |
| 6 | SYSTIMER | Reemplazar busy-wait por temporizador | ★★★ | Rutina de delay precisa |
| 7 | Interrupciones | Timer que togglea LED sin busy-wait | ★★★ | Handler + mtvec |
| 8 | Performance | Medir jitter busy-wait vs interrupción | ★★★ | Informe comparativo |
| 9 | PWM Software | Generar duty-cycle configurable | ★★★ | Función pwm_soft() |
| 10 | Refactor | Modularizar (gpio.c, timer.c, uart.c) | ★★ | Árbol limpio + doc |

## Ejercicio 1: GPIO y Frecuencia

1. Seleccionar otro GPIO seguro (no strapping). Ej: GPIO5.
2. Ajustar `LED_GPIO` y recompilar.
3. Hacer parpadear a ~1 Hz (ajustar delay).
4. Documentar: ¿cómo estimaste el delay? ¿Qué limita tu precisión?

Criterio de evaluación:

- Código compila.
- Explicación clara de elección de delay.
- Captura (foto/logic analyzer opcional) si disponible.

## Ejercicio 2: Registro mcycle

Objetivo: medir periodos reales.

1. Leer CSR `mcycle` antes de poner LED alto y antes de repetir ciclo.
2. Almacenar diferencia en un array pequeño (5 muestras) y calcular promedio.
3. Presentar cálculo comparando vs estimación nominal (NOP ~1 ciclo, considerar pipeline/overheads mínimos).

Hints RISC-V:

```c
static inline uint64_t mcycle(void){
    uint32_t lo, hi1, hi2;
    __asm__ volatile ("rdcycleh %0" : "=r"(hi1));
    __asm__ volatile ("rdcycle %0"  : "=r"(lo));
    __asm__ volatile ("rdcycleh %0" : "=r"(hi2));
    if(hi1!=hi2){
        __asm__ volatile ("rdcycle %0"  : "=r"(lo));
    }
    return ((uint64_t)hi2<<32)|lo;
}
```

No uses stdio; puedes (temporal) almacenar en variables globales y observar con debugger.

## Ejercicio 3: delay_cycles()

Crear `delay_cycles(uint32_t n)` que use un loop en ensamblador minimizando overhead.

- Comparar con el delay original NOP a NOP.
- Documentar diferencias en precisión.

## Ejercicio 4: UART Mínima

1. Investigar dirección base UART0 (ESP32-C3 TRM) y registros TXFIFO / status.
2. Inicializar baud (divisor) para 115200 (documentar fórmula).
3. Implementar `uart_putc(ch)` bloqueante y enviar "Hola" en loop.
4. Añadir un pequeño retardo entre caracteres.

Criterios:

- Explicación divisor baud.
- Código comentado.

## Ejercicio 5: Botón (Input)

1. Elegir pin con pulsador externo (configurar resistencia pull-up/pull-down manual).
2. Leer estado en cada iteración y cambiar frecuencia de parpadeo.
3. Implementar simple anti-rebote por delay corto o contador.

## Ejercicio 6: SYSTIMER como Delay

1. Configurar SYSTIMER para contar microsegundos (leer TRM: comparadores y registros de valor).
2. Implementar `delay_us(x)` sin busy-wait de NOPs (o con polling al comparador).
3. Mostrar diferencia en jitter midiendo con analizador lógico.

## Ejercicio 7: Interrupción de Timer

1. Definir vector mtvec (modo direct) apuntando a un handler.
2. Configurar una fuente de interrupción periódica (ej: timer o systimer comparador) que togglee LED.
3. Medir jitter vs busy-wait.

## Ejercicio 8: Análisis de Jitter

- Crear tabla: (método, periodo medio, desviación, max desvío).
- Conclusión: ¿qué aporta la interrupción frente a busy-wait?

## Ejercicio 9: PWM Software

1. Crear bucle con contadores que genere duty configurable (ej: 25%, 50%, 75%).
2. Ajustar resolución (ej: 256 pasos) y medir frecuencia final.
3. Explicar limitaciones (CPU ocupada, jitter, escalabilidad).

## Ejercicio 10: Refactor Modular

- Separar: gpio.c/.h, timer.c/.h, uart.c/.h.
- Añadir encabezados con prototipos y comentarios Doxygen básicos.
- Mantener binario lo más pequeño posible (comentar cambios de tamaño).

## Criterios Globales de Evaluación

| Aspecto | Peso |
|---------|------|
| Comprensión técnica (explicaciones) | 30% |
| Correctitud funcional | 25% |
| Claridad de código / comentarios | 20% |
| Rigor en mediciones (cuando aplica) | 15% |
| Uso disciplinado de control versiones | 10% |

## Sugerencias de Buenas Prácticas

- Medir antes de optimizar.
- Introducir una sola variable experimental por commit.
- Anotar siempre: frecuencia CPU asumida.
- Evitar números mágicos: usar defines con nombre.

## Extensiones (Opcional)

- Implementar logging UART con formato hex de registros de GPIO.
- Mini bootloader propio que salte a la app (conceptual).
- Comparar footprint con versión equivalente en ESP-IDF.

---
Fin de la guía inicial. Continúa iterando con disciplina y midiendo cada cambio.
