# Proyecto Bare-Metal ESP32-C3  
PWM + ADC + UART + HC-SR04 + GPIO Matrix

Este proyecto implementa un firmware **100% bare-metal** (sin ESP-IDF ni RTOS) sobre un **ESP32-C3 DevKitC-02**, configurando todos los periféricos directamente a través de registros.

Incluye:

- Control de **3 LEDs**:  
  - GPIO3 (verde)  
  - GPIO6 (azul)  
  - GPIO8 (rojo)
- **PWM (LEDC)** en GPIO3
- **ADC SAR** (lectura única) en GPIO0 para un potenciómetro
- **Sensor ultrasónico HC-SR04**  
  - TRIG → GPIO5  
  - ECHO → GPIO2
- **UART0** TX por GPIO21 (debug)
- **Botón** en GPIO10 con flanco ascendente
- Deshabilitación manual de watchdogs
- Control directo de **GPIO**, **IO_MUX**, **GPIO Matrix**, **Timers**, **Clocks**, **PWM**, **ADC**, **UART**

---

## Comportamiento General

### 1. Botón (GPIO10)
Se detecta **flanco ascendente**:

- `button_state = 1`  
  → **Modo apagado:** PWM OFF y LEDs apagados  
- `button_state = 0`  
  → **Modo activo**

---

## Lógica en Modo Activo

En cada ciclo del bucle principal:

1. Se lee el potenciómetro:  
   `sample = adc_sample_once()` (0–4095)

2. Se genera un offset dinámico:  
   `dynamic_offset = sample * 2`

3. Se mide el pulso del HC-SR04:  
   `pulse = hcsr04_measure_pulse(dynamic_offset)`

### Reglas para los LEDs

| Condición | LED Verde (GPIO3, PWM) | LED Azul (GPIO6) | LED Rojo (GPIO8) | Significado |
|----------|-------------------------|------------------|-------------------|-------------|
| `button_state = 1` | OFF | OFF | OFF | Modo apagado |
| `pulse == 0` | PWM proporcional | ON | OFF | Objeto detectado |
| `pulse != 0` | OFF | OFF | ON | No hay objeto |

---

## Periféricos Configurados

### GPIO + IO_MUX
Todos los pines se configuran a mano:  
- Activación de IE/OE  
- Pull-ups / pull-downs manuales  
- MCU_SEL = GPIO  
- LIMPIEZA completa de alternativas de función

### ADC SAR
- Modo **oneshot**  
- Canal 0  
- Atenuación 11 dB  
- Lectura por bandera `APB_SARADC_ADC1_DONE_INT_ST`

### PWM LEDC
- Timer LS0  
- Canal 0  
- Frecuencia 2 kHz  
- Resolución 10 bits  
- Duty actualizado por registro

### UART0
- TX → GPIO21  
- RX opcional en GPIO20  
- 115200 8N1  
- FIFO manejado a mano mediante registros

### HC-SR04
- TRIG: pulso de 10 µs generado por busy-wait  
- ECHO: ancho de pulso medido por loop controlado por CPU  
- Lógica de timeout configurable dinámicamente

---

## Compilación y Flash

### Requisitos
- Toolchain ESP32-C3: 
- `startup.s`
- `make`
- `flash.sh`
- `build.sh`

### Compilar
```bash
make
