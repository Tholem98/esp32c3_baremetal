#include <stdint.h>
#include "wdtfix.h"

#define BIT(n) (1U << (n))                    // Máscara de un bit
#define REG32(addr) (*(volatile uint32_t *)(addr)) // Acceso directo a registro de 32 bits

#define DR_REG_GPIO_BASE        0x60004000UL  // Base periférico GPIO
#define DR_REG_IO_MUX_BASE      0x60009000UL  // Base IO_MUX (selección de función/pulls)
#define DR_REG_SYSTEM_BASE      0x600C0000UL  // Base registro de sistema (clocks/resets)
#define DR_REG_APB_SARADC_BASE  0x60040000UL  // Base ADC SAR digital
#define DR_REG_LEDC_BASE        0x60019000UL  // Base bloque LEDC (PWM hardware)

#define GPIO_OUT_W1TS_REG   (DR_REG_GPIO_BASE + 0x0008)  // Set pin high (write-1-to-set)
#define GPIO_OUT_W1TC_REG   (DR_REG_GPIO_BASE + 0x000C)  // Set pin low  (write-1-to-clear)
#define GPIO_ENABLE_W1TS_REG (DR_REG_GPIO_BASE + 0x0024) // Habilitar OE
#define GPIO_ENABLE_W1TC_REG (DR_REG_GPIO_BASE + 0x0028) // Deshabilitar OE

#define GPIO_IN_REG         (DR_REG_GPIO_BASE + 0x003C)   // <--- REGISTRO DE ENTRADA GPIO

#define IO_MUX_GPIO0_REG    (DR_REG_IO_MUX_BASE + 0x0004) // IO_MUX para GPIO0 (ADC)
#define IO_MUX_GPIO3_REG    (DR_REG_IO_MUX_BASE + 0x0010) // IO_MUX para GPIO3 (LED)
#define IO_MUX_FUN_IE       BIT(9)   // Input enable digital
#define IO_MUX_FUN_PU       BIT(8)   // Pull-up digital
#define IO_MUX_FUN_PD       BIT(7)   // Pull-down digital
#define IO_MUX_MCU_SEL_MASK (0x7U << 12) // Selector de función
#define IO_MUX_MCU_SEL_GPIO 1U       // Función GPIO
#define IO_MUX_GPIO2_REG        (DR_REG_IO_MUX_BASE + 0x000C)
#define IO_MUX_GPIO4_REG        (DR_REG_IO_MUX_BASE + 0x0014)
#define IO_MUX_GPIO5_REG        (DR_REG_IO_MUX_BASE + 0x0018)

#define SYSTEM_PERIP_CLK_EN0_REG (DR_REG_SYSTEM_BASE + 0x0010) // Registro de clocks
#define SYSTEM_PERIP_RST_EN0_REG (DR_REG_SYSTEM_BASE + 0x0018) // Registro de resets
#define SYSTEM_APB_SARADC_CLK_EN BIT(28) // Bit de clock para ADC SAR
#define SYSTEM_APB_SARADC_RST    BIT(28) // Bit de reset para ADC SAR
#define SYSTEM_LEDC_CLK_EN       BIT(11) // Bit de clock para LEDC
#define SYSTEM_LEDC_RST          BIT(11) // Bit de reset para LEDC

#define APB_SARADC_CTRL_REG            (DR_REG_APB_SARADC_BASE + 0x0000) // Control general ADC
#define APB_SARADC_START_FORCE         BIT(0)  // Forzar arranque digital
#define APB_SARADC_START               BIT(1)  // Señal start SW
#define APB_SARADC_SAR_CLK_GATED       BIT(6)  // Clock gated para SAR
#define APB_SARADC_SAR_CLK_DIV_S       7       // Shift divisor clock
#define APB_SARADC_SAR_CLK_DIV_M       (0xFFU << APB_SARADC_SAR_CLK_DIV_S)
#define APB_SARADC_XPD_SAR_FORCE_S     27      // Shift modo power
#define APB_SARADC_XPD_SAR_FORCE_M     (0x3U << APB_SARADC_XPD_SAR_FORCE_S)

#define APB_SARADC_ONETIME_SAMPLE_REG  (DR_REG_APB_SARADC_BASE + 0x0020) // Control oneshot
#define APB_SARADC1_ONETIME_SAMPLE     BIT(31) // Selecciona ADC1
#define APB_SARADC_ONETIME_START       BIT(29) // Lanzar conversión
#define APB_SARADC_ONETIME_CHANNEL_S   25      // Shift canal
#define APB_SARADC_ONETIME_CHANNEL_M   (0xFU << APB_SARADC_ONETIME_CHANNEL_S)
#define APB_SARADC_ONETIME_ATTEN_S     23      // Shift atenuación
#define APB_SARADC_ONETIME_ATTEN_M     (0x3U << APB_SARADC_ONETIME_ATTEN_S)

#define APB_SARADC_1_DATA_STATUS_REG   (DR_REG_APB_SARADC_BASE + 0x002C) // Resultado ADC1

#define APB_SARADC_INT_ENA_REG         (DR_REG_APB_SARADC_BASE + 0x0040) // Enable de flags
#define APB_SARADC_ADC1_DONE_INT_ENA   BIT(31) // Habilita flag ADC1 done
#define APB_SARADC_INT_ST_REG          (DR_REG_APB_SARADC_BASE + 0x0048) // Estado de flags
#define APB_SARADC_ADC1_DONE_INT_ST    BIT(31) // Flag ADC1 conversión terminada
#define APB_SARADC_INT_CLR_REG         (DR_REG_APB_SARADC_BASE + 0x004C) // Clear de flags
#define APB_SARADC_ADC1_DONE_INT_CLR   BIT(31) // Limpia flag done

#define LEDC_LSTIMER0_CONF_REG   (DR_REG_LEDC_BASE + 0x00A0)
#define LEDC_LSTIMER0_PARA_UP    BIT(25)
#define LEDC_LSTIMER0_RST        BIT(23)
#define LEDC_LSTIMER0_PAUSE      BIT(22)
#define LEDC_CLK_DIV_LSTIMER0_M  ((0x0003FFFFU) << 4)
#define LEDC_CLK_DIV_LSTIMER0_S  4
#define LEDC_LSTIMER0_DUTY_RES_M ((0xFU) << 0)
#define LEDC_LSTIMER0_DUTY_RES_S 0

#define LEDC_CONF_REG            (DR_REG_LEDC_BASE + 0x00D0)
#define LEDC_CLK_EN              BIT(31)
#define LEDC_APB_CLK_SEL_M       ((0x3U) << 0)
#define LEDC_APB_CLK_SEL_S       0
#define LEDC_APB_CLK_SEL_APB     1U

#define LEDC_LSCH0_CONF0_REG     (DR_REG_LEDC_BASE + 0x0000)
#define LEDC_PARA_UP_LSCH0       BIT(4)
#define LEDC_IDLE_LV_LSCH0       BIT(3)
#define LEDC_SIG_OUT_EN_LSCH0    BIT(2)
#define LEDC_TIMER_SEL_LSCH0_M   ((0x3U) << 0)
#define LEDC_TIMER_SEL_LSCH0_S   0

#define LEDC_LSCH0_HPOINT_REG    (DR_REG_LEDC_BASE + 0x0004)
#define LEDC_LSCH0_DUTY_REG      (DR_REG_LEDC_BASE + 0x0008)

#define LEDC_LSCH0_CONF1_REG     (DR_REG_LEDC_BASE + 0x000C)
#define LEDC_DUTY_START_LSCH0    BIT(31)

#define GPIO_FUNC3_OUT_SEL_CFG_REG (DR_REG_GPIO_BASE + 0x0560)
#define GPIO_FUNC3_OEN_INV_SEL      BIT(10)
#define GPIO_FUNC3_OEN_SEL          BIT(9)
#define GPIO_FUNC3_OUT_INV_SEL      BIT(8)
#define GPIO_FUNC3_OUT_SEL_M        ((0xFFU) << 0)
#define GPIO_FUNC3_OUT_SEL_S        0

#define LEDC_LS_SIG_OUT0_IDX    45U  // Señal PWM canal 0 (low-speed)

#define LED_GPIO        3U
#define LED2_GPIO       5U
#define POT_GPIO        0U
#define BUTTON_GPIO     2U   // <---- Pin de entrada Boton y ECHO
#define BUTTON_MASK     BIT(BUTTON_GPIO)
#define LED_MASK        BIT(LED_GPIO)
#define LED2_MASK       BIT(LED2_GPIO)
#define POT_MASK        BIT(POT_GPIO)
#define TRIG_GPIO       5U      // TRIG del HC-SR04
#define ECHO_GPIO       2U      // ECHO del HC-SR04 (con divisor a 3.3V) entrada
#define TRIG_MASK       BIT(TRIG_GPIO)
#define ECHO_MASK       BIT(ECHO_GPIO)

// HC-SR04
#define HCSR04_TIMEOUT       40000U   // Iteraciones máx esperando
#define HCSR04_NEAR_THRESHOLD 300U   // Umbral "cerca" (ajustable)


#define ADC_ATTEN_11DB  3U
#define ADC_THRESHOLD   1630U
#define LOOP_DELAY      5000U
#define ADC_MAX_RANGE       (4095U) // Rango máximo del ADC (12 bits)
#define ADC_ZERO_BIAS   1630U   // Cuentas residuales con cursor a GND (ajustar según hardware)

// Umbral mínimo de pulso (aprox. 1.7 cm). 
// Esto asegura que al mínimo del potenciómetro la detección sea muy cercana.
#define MIN_PULSE_THRESHOLD (1000U) 
// Umbral máximo de pulso para 50 cm. Este valor ha sido AUMENTADO.
// Si tu CPU es lenta, el 'count' para 50cm podría ser muy grande.
#define MAX_PULSE_THRESHOLD (5000U) 
// Se calcula el rango que el potenciómetro puede cubrir.
#define PULSE_RANGE         (MAX_PULSE_THRESHOLD - MIN_PULSE_THRESHOLD)

#define LEDC_PWM_FREQ_HZ       2000ULL
#define LEDC_TIMER_RES_BITS    10U
#define LEDC_TIMER_SOURCE_HZ   80000000ULL
#define LEDC_CLK_DIV_FRAC_BITS 8U
#define LEDC_TIMER_DIVIDER_NUM (LEDC_TIMER_SOURCE_HZ << LEDC_CLK_DIV_FRAC_BITS)
#define LEDC_TIMER_DIVIDER_DEN (LEDC_PWM_FREQ_HZ * (1ULL << LEDC_TIMER_RES_BITS))
#define LEDC_TIMER_DIVIDER ((uint32_t)(LEDC_TIMER_DIVIDER_NUM / LEDC_TIMER_DIVIDER_DEN))
#define LEDC_DUTY_MAX        ((1U << LEDC_TIMER_RES_BITS) - 1U)
#define LEDC_DUTY_SHIFT      4U

#if ((LEDC_TIMER_DIVIDER_NUM / LEDC_TIMER_DIVIDER_DEN) == 0) || ((LEDC_TIMER_DIVIDER_NUM / LEDC_TIMER_DIVIDER_DEN) > 0x3FFFFU)
#error "LEDC_TIMER_DIVIDER fuera de rango para el campo de 18 bits"
#endif


#define DR_REG_UART_BASE(i)     (0x60000000UL + (0x1000 * (i))) // Base para UART0 (i=0) y UART1 (i=1)

#define DR_REG_UART0_BASE       DR_REG_UART_BASE(0) // 0x60000000

#define UART_CLK_DIV_REG(i)     (DR_REG_UART_BASE(i) + 0x0014) // Divisor de clock (baud rate)
#define UART_FIFO_REG(i)        (DR_REG_UART_BASE(i) + 0x0000) // Registro de datos/FIFO

#define UART_STATUS_REG(i)      (DR_REG_UART_BASE(i) + 0x001C)// Registro de estado (para TX)
#define UART_TXFIFO_CNT_S       16// Shift para contador FIFO
#define UART_TXFIFO_CNT_M       (0x1FFU << UART_TXFIFO_CNT_S)// Máscara
#define UART_FIFO_SIZE          0x7FU// Tamaño del FIFO (128 bytes)

#define SYSTEM_UART_CLK_EN(i)   (1U << (i))       // i=0 para UART0, i=1 para UART1
#define SYSTEM_UART_RST(i)      (1U << (i))

// Base y Máscaras
#define GPIO_FUNC_OUT_SEL_S     0       // Shift para el selector de función de salida
#define IO_MUX_MCU_SEL_V        1U      // Valor 1 para seleccionar GPIO matrix (no función default)

// Registros de la matriz de función (para conectar UART0 a GPIOs)
#define GPIO_FUNC21_OUT_SEL_CFG_REG (DR_REG_GPIO_BASE + 0x05BC) // Registro de salida para GPIO21
#define GPIO_FUNC20_IN_SEL_CFG_REG  (DR_REG_GPIO_BASE + 0x05CC) // Registro de entrada para GPIO20

// Índices de la señal UART0 en la matriz
#define U0TXD_OUT_IDX                  0x00U  // Índice de la señal U0TXD
#define U0RXD_IN_IDX                   0x00U  // Índice de la señal U0RXD (este es el default, pero lo ponemos)

// Registros IO_MUX específicos para GPIO21 y GPIO20
#define IO_MUX_GPIO21_REG       (DR_REG_IO_MUX_BASE + 0x0058)
#define IO_MUX_GPIO20_REG       (DR_REG_IO_MUX_BASE + 0x0054)

// Definiciones adicionales para GPIO/IO_MUX para asignar pines
#define UART0_TX_GPIO 21U
#define UART0_RX_GPIO 20U
#define UART0_TX_OUT_IDX 0x00U // Seleccion de funcion de salida
#define UART0_RX_IN_IDX 0x00U  // Seleccion de funcion de entrada

//TIMER
#define DR_REG_TIMG_BASE(i)     (0x60082000UL + (0x1000 * (i)))
#define DR_REG_TIMG0_BASE       DR_REG_TIMG_BASE(0) // Base del Timer Group 0

//#define SYSTEM_PERIP_CLK_EN0_REG  (DR_REG_SYSTEM_BASE + 0x10)
//#define SYSTEM_PERIP_RST_EN0_REG  (DR_REG_SYSTEM_BASE + 0x0C)

#define SYSTEM_TIMERGROUP0_CLK_EN BIT(24)
#define SYSTEM_TIMERGROUP0_RST    BIT(24)

#define TIMG_T0CONFIG_REG       (DR_REG_TIMG0_BASE + 0x0000)
#define TIMG_T0_EN              BIT(31)     // Habilitar Timer
#define TIMG_T0_DIVIDER_S       16          // Shift para el divisor de clock
#define TIMG_T0_INCREASE        BIT(13)     // Contar hacia arriba
#define TIMG_T0_AUTORELOAD      BIT(19)     // Deshabilitamos autoreload
#define TIMG_T0_CLK_SRC_S        29          // Shift Clock Source

#define TIMG_T0LOAD_REG         (DR_REG_TIMG0_BASE + 0x0008) // Cargar valor inicial
#define TIMG_T0LOAD_LOW_REG     (DR_REG_TIMG0_BASE + 0x0004) // Contador bajo (32 bits)
#define TIMG_T0LOAD_HIGH_REG    (DR_REG_TIMG0_BASE + 0x000c) // Contador alto (16 bits)
#define TIMG_T0_UPDATE_REG      (DR_REG_TIMG0_BASE + 0x0010) // Forzar actualización de lectura
#define TIMG_T0_LOAD_EN         BIT(31)     // Habilitar la carga

#define TIMG_T0_CNT_LOW_REG     (DR_REG_TIMG0_BASE + 0x0004) // Leer contador bajo
#define TIMG_T0_CNT_HIGH_REG    (DR_REG_TIMG0_BASE + 0x000c) // Leer contador alto

#define TIMG_T0_UPDATE  BIT(31) // bit de update de lectura

// Clock fuente es APB_CLK (80 MHz)
#define TIMG_DIVIDER_US         80U         // 80 MHz / 80 = 1 MHz (1 tick = 1 µs)

static void ledc_set_duty(uint32_t duty);

static uint64_t timer_get_us(void);

static void gpio_init(void) {
    // GPIO3 queda como salida controlada por LEDC (sin pulls, función GPIO)
    uint32_t reg = REG32(IO_MUX_GPIO3_REG);
    reg &= ~(IO_MUX_FUN_IE | IO_MUX_FUN_PU | IO_MUX_FUN_PD | IO_MUX_MCU_SEL_MASK);
    reg |= (IO_MUX_MCU_SEL_GPIO << 12);
    REG32(IO_MUX_GPIO3_REG) = reg;
    REG32(GPIO_ENABLE_W1TS_REG) = LED_MASK;

    uint32_t reg5 = REG32(IO_MUX_GPIO5_REG);
    reg5 &= ~(IO_MUX_FUN_IE | IO_MUX_FUN_PU | IO_MUX_FUN_PD | IO_MUX_MCU_SEL_MASK);
    reg5 |= (IO_MUX_MCU_SEL_GPIO << 12);
    REG32(IO_MUX_GPIO5_REG) = reg5;
    REG32(GPIO_ENABLE_W1TS_REG) = TRIG_MASK;
    

    //REG32(IO_MUX_GPIO0_REG) = 0U;

    // GPIO0 en modo analógico (sin OE ni pulls) para el potenciómetro
    reg = REG32(IO_MUX_GPIO0_REG);
    reg &= ~(IO_MUX_FUN_IE | IO_MUX_FUN_PU | IO_MUX_FUN_PD | IO_MUX_MCU_SEL_MASK);
    REG32(IO_MUX_GPIO0_REG) = reg;
    REG32(GPIO_ENABLE_W1TC_REG) = POT_MASK;

    // CONFIGURACION PARA ECHO (GPIO2) --- ENTRADA DIGITAL
    uint32_t reg2 = REG32(IO_MUX_GPIO2_REG);
    reg2 &= ~(IO_MUX_FUN_PU | IO_MUX_FUN_PD | IO_MUX_MCU_SEL_MASK);
    reg2 |= IO_MUX_FUN_IE; // habilitar entrada digital
    reg2 |= IO_MUX_FUN_PD;
    reg2 |= (IO_MUX_MCU_SEL_GPIO << 12);
    REG32(IO_MUX_GPIO2_REG) = reg2;
    // Asegurarse que sea entrada (deshabilitar OE)
    REG32(GPIO_ENABLE_W1TC_REG) = ECHO_MASK;

    // Asegurar que NO sea salida (entrada pura)
    //REG32(GPIO_ENABLE_W1TC_REG) = BUTTON_MASK;
    // Deshabilitar OE: ECHO/BUTTON (GPIO2) debe ser una entrada pura
    //REG32(GPIO_ENABLE_W1TC_REG) = ECHO_MASK;

    
}

static void adc_init(void) {
    // Clock/reset del SARADC
    REG32(SYSTEM_PERIP_CLK_EN0_REG) |= SYSTEM_APB_SARADC_CLK_EN;
    REG32(SYSTEM_PERIP_RST_EN0_REG) |= SYSTEM_APB_SARADC_RST;
    REG32(SYSTEM_PERIP_RST_EN0_REG) &= ~SYSTEM_APB_SARADC_RST;

    // Forzar ADC encendido, activar clock y fijar divisor
    uint32_t ctrl = REG32(APB_SARADC_CTRL_REG);
    ctrl |= APB_SARADC_SAR_CLK_GATED;
    ctrl &= ~APB_SARADC_XPD_SAR_FORCE_M;
    ctrl |= (3U << APB_SARADC_XPD_SAR_FORCE_S);
    ctrl &= ~APB_SARADC_SAR_CLK_DIV_M;
    ctrl |= (4U << APB_SARADC_SAR_CLK_DIV_S);
    ctrl &= ~(APB_SARADC_START_FORCE | APB_SARADC_START);
    REG32(APB_SARADC_CTRL_REG) = ctrl;

    // Configurar canal 0 con atenuación 11 dB (full scale ~3.3 V)
    uint32_t sample = REG32(APB_SARADC_ONETIME_SAMPLE_REG);
    sample |= APB_SARADC1_ONETIME_SAMPLE;
    sample &= ~APB_SARADC_ONETIME_CHANNEL_M;
    sample |= (0U << APB_SARADC_ONETIME_CHANNEL_S);
    sample &= ~APB_SARADC_ONETIME_ATTEN_M;
    sample |= (ADC_ATTEN_11DB << APB_SARADC_ONETIME_ATTEN_S);
    sample &= ~APB_SARADC_ONETIME_START;
    REG32(APB_SARADC_ONETIME_SAMPLE_REG) = sample;

    // Habilitar y limpiar flag de conversión terminada
    REG32(APB_SARADC_INT_ENA_REG) |= APB_SARADC_ADC1_DONE_INT_ENA;
    REG32(APB_SARADC_INT_CLR_REG) = APB_SARADC_ADC1_DONE_INT_CLR;
}

static void ledc_init(void) {
    // Activar clock/reset de LEDC
    REG32(SYSTEM_PERIP_CLK_EN0_REG) |= SYSTEM_LEDC_CLK_EN;
    REG32(SYSTEM_PERIP_RST_EN0_REG) |= SYSTEM_LEDC_RST;
    REG32(SYSTEM_PERIP_RST_EN0_REG) &= ~SYSTEM_LEDC_RST;

    // Seleccionar reloj APB (80 MHz) y habilitar módulo
    uint32_t ledc_conf = REG32(LEDC_CONF_REG);
    ledc_conf |= LEDC_CLK_EN;
    ledc_conf &= ~LEDC_APB_CLK_SEL_M;
    ledc_conf |= (LEDC_APB_CLK_SEL_APB << LEDC_APB_CLK_SEL_S);
    REG32(LEDC_CONF_REG) = ledc_conf;

    // Configurar temporizador low-speed 0: resolución y divisor fraccionario (8 bits fracc.)
    uint32_t timer_conf = REG32(LEDC_LSTIMER0_CONF_REG);
    timer_conf &= ~(LEDC_CLK_DIV_LSTIMER0_M | LEDC_LSTIMER0_DUTY_RES_M | LEDC_LSTIMER0_PAUSE);
    timer_conf |= ((LEDC_TIMER_DIVIDER << LEDC_CLK_DIV_LSTIMER0_S) & LEDC_CLK_DIV_LSTIMER0_M);
    timer_conf |= ((LEDC_TIMER_RES_BITS << LEDC_LSTIMER0_DUTY_RES_S) & LEDC_LSTIMER0_DUTY_RES_M);
    REG32(LEDC_LSTIMER0_CONF_REG) = timer_conf;
    REG32(LEDC_LSTIMER0_CONF_REG) |= LEDC_LSTIMER0_RST;
    REG32(LEDC_LSTIMER0_CONF_REG) &= ~LEDC_LSTIMER0_RST;
    REG32(LEDC_LSTIMER0_CONF_REG) |= LEDC_LSTIMER0_PARA_UP;

    // Inicializar canal 0: duty 0, usa timer 0, habilita salida
    REG32(LEDC_LSCH0_HPOINT_REG) = 0;
    REG32(LEDC_LSCH0_DUTY_REG) = 0;
    uint32_t ch0_conf0 = REG32(LEDC_LSCH0_CONF0_REG);
    ch0_conf0 &= ~(LEDC_TIMER_SEL_LSCH0_M | LEDC_IDLE_LV_LSCH0 | LEDC_SIG_OUT_EN_LSCH0);
    ch0_conf0 |= LEDC_SIG_OUT_EN_LSCH0; // Timer 0 (valor 0)
    REG32(LEDC_LSCH0_CONF0_REG) = ch0_conf0;
    REG32(LEDC_LSCH0_CONF0_REG) |= LEDC_PARA_UP_LSCH0;
    uint32_t ch0_conf1 = REG32(LEDC_LSCH0_CONF1_REG);
    ch0_conf1 |= LEDC_DUTY_START_LSCH0;
    REG32(LEDC_LSCH0_CONF1_REG) = ch0_conf1;

    // Conectar señal LEDC canal 0 a GPIO3
    uint32_t func3 = REG32(GPIO_FUNC3_OUT_SEL_CFG_REG);
    func3 &= ~(GPIO_FUNC3_OEN_INV_SEL | GPIO_FUNC3_OEN_SEL | GPIO_FUNC3_OUT_INV_SEL | GPIO_FUNC3_OUT_SEL_M);
    func3 |= (LEDC_LS_SIG_OUT0_IDX << GPIO_FUNC3_OUT_SEL_S) & GPIO_FUNC3_OUT_SEL_M;
    REG32(GPIO_FUNC3_OUT_SEL_CFG_REG) = func3;

    ledc_set_duty(0);
}

static void timer_init(void) {

REG32(SYSTEM_PERIP_CLK_EN0_REG) |= SYSTEM_TIMERGROUP0_CLK_EN;
    REG32(SYSTEM_PERIP_RST_EN0_REG) &= ~SYSTEM_TIMERGROUP0_RST;

    uint32_t cfg = REG32(TIMG_T0CONFIG_REG);
    cfg &= ~TIMG_T0_EN;                 // apagar timer
    cfg &= ~(0xFF << TIMG_T0_DIVIDER_S);
    cfg |= (TIMG_DIVIDER_US << TIMG_T0_DIVIDER_S);
    cfg |= TIMG_T0_INCREASE;            // contar hacia arriba
    cfg &= ~TIMG_T0_AUTORELOAD;
    cfg &= ~(3 << TIMG_T0_CLK_SRC_S);
    cfg |= (1 << TIMG_T0_CLK_SRC_S);    // APB_CLK
    REG32(TIMG_T0CONFIG_REG) = cfg;

    REG32(TIMG_T0LOAD_REG) = 0;         // inicializar contador
    REG32(TIMG_T0_UPDATE_REG) = TIMG_T0_LOAD_EN; // solo 1 vez para cargar 0

    cfg |= TIMG_T0_EN;                  // encender timer
    REG32(TIMG_T0CONFIG_REG) = cfg;
}



static void uart_init(void) {
    /*// --- 1. Activar Clock y Reset UART0 ---
    REG32(SYSTEM_PERIP_CLK_EN0_REG) |= SYSTEM_UART_CLK_EN(0);
    REG32(SYSTEM_PERIP_RST_EN0_REG) |= SYSTEM_UART_RST(0);
    REG32(SYSTEM_PERIP_RST_EN0_REG) &= ~SYSTEM_UART_RST(0);

    // --- 2. Configurar Baud Rate (115200) ---
    // Divisor = 40MHz / 115200 = 347.22 (usamos 347, que es 0x15B)
    REG32(UART_CLK_DIV_REG(0)) = (347U << 4); 

    // --- 3. Configurar Pines (GPIO21=TX, GPIO20=RX) ---
    
    // 3a. Mapear UART0 TX a GPIO21
    // Conectar U0TXD_OUT_IDX (0x00) al GPIO21
    REG32(GPIO_FUNC21_OUT_SEL_CFG_REG) = (U0TXD_OUT_IDX << GPIO_FUNC_OUT_SEL_S);
    // Habilitar la salida (OE) para GPIO21
    REG32(GPIO_ENABLE_W1TS_REG) = BIT(UART0_TX_GPIO);
    // Configurar IO_MUX para usar la Matriz GPIO
    REG32(IO_MUX_GPIO21_REG) &= ~(IO_MUX_MCU_SEL_MASK); 
    REG32(IO_MUX_GPIO21_REG) |= (IO_MUX_MCU_SEL_V << 12); 

    // 3b. Mapear UART0 RX a GPIO20
    // Conectar U0RXD_IN_IDX (0x00) al GPIO20 (con el bit de habilitación de entrada)
    REG32(GPIO_FUNC20_IN_SEL_CFG_REG) = (U0RXD_IN_IDX << GPIO_FUNC_OUT_SEL_S) | IO_MUX_FUN_IE;
    // Deshabilitar la salida (OE) para GPIO20
    REG32(GPIO_ENABLE_W1TC_REG) = BIT(UART0_RX_GPIO);
    // Configurar IO_MUX para usar la Matriz GPIO
    REG32(IO_MUX_GPIO20_REG) &= ~(IO_MUX_MCU_SEL_MASK);
    REG32(IO_MUX_GPIO20_REG) |= (IO_MUX_MCU_SEL_V << 12); 
    
    // Nota: Configuración de palabra (8 bits, sin paridad, 1 bit de parada) es el default y se omite por simplicidad.
    */
        // Solo configurar baud rate: USB-Serial usa UART0 internamente
    REG32(UART_CLK_DIV_REG(0)) = (347U << 4); // 115200 baud
}

static uint16_t adc_sample_once(void) {
    // Pulso de start (low→high) para disparar conversión oneshot
    uint32_t sample = REG32(APB_SARADC_ONETIME_SAMPLE_REG);
    sample &= ~APB_SARADC_ONETIME_START;
    REG32(APB_SARADC_ONETIME_SAMPLE_REG) = sample;
    for (volatile uint32_t i = 0; i < 32; ++i) {
        __asm__ volatile("nop");
    }
    sample |= APB_SARADC_ONETIME_START;
    REG32(APB_SARADC_ONETIME_SAMPLE_REG) = sample;

    while ((REG32(APB_SARADC_INT_ST_REG) & APB_SARADC_ADC1_DONE_INT_ST) == 0U) {
    }

    // Capturo 12 bits útiles y limpio flag
    uint32_t raw = REG32(APB_SARADC_1_DATA_STATUS_REG) & 0x1FFFFU;
    REG32(APB_SARADC_INT_CLR_REG) = APB_SARADC_ADC1_DONE_INT_CLR;
    return (uint16_t)(raw & 0x0FFFU);
}

static void short_delay(void) {
    // Busy-wait simple (no timers configurados)
    for (volatile uint32_t i = 0; i < LOOP_DELAY; ++i) {
        __asm__ volatile("nop");
    }
}

static void ledc_set_duty(uint32_t duty) {
    if (duty > LEDC_DUTY_MAX) {
        duty = LEDC_DUTY_MAX;
    }
    REG32(LEDC_LSCH0_DUTY_REG) = duty << LEDC_DUTY_SHIFT;
    REG32(LEDC_LSCH0_CONF1_REG) |= LEDC_DUTY_START_LSCH0;
    REG32(LEDC_LSCH0_CONF0_REG) |= LEDC_PARA_UP_LSCH0;
}

static void uart_putc(char c) {
    // Esperar hasta que el FIFO no esté lleno
    while ((REG32(UART_STATUS_REG(0)) & UART_TXFIFO_CNT_M) >= (UART_FIFO_SIZE << UART_TXFIFO_CNT_S)) {
        // Busy-wait
        __asm__ volatile("nop");
    }
    
    // Escribir el carácter al registro FIFO (dirección 0x60000000)
    REG32(UART_FIFO_REG(0)) = (uint32_t)c;
}

static void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

static uint64_t timer_get_us(void) {

    REG32(TIMG_T0_UPDATE_REG) = TIMG_T0_UPDATE; // SOLO ESTO
uint32_t low = REG32(TIMG_T0_CNT_LOW_REG);
uint32_t high = REG32(TIMG_T0_CNT_HIGH_REG);
return ((uint64_t)high << 32) | low;
}



// ----------------------------------------
// Medir pulso del HC-SR04 (ECHO)
// Devuelve "cuántas iteraciones" estuvo en alto
// ----------------------------------------
static void tiny_delay(void) {
    for (volatile uint32_t i = 0; i < 200; ++i) {
        __asm__ volatile("nop");
    }
}

/*
static uint32_t hcsr04_measure_pulse(void) {

    uint64_t start_time = 0;
    uint64_t end_time = 0;
    uint32_t timeout = 0; // Usaremos el timeout para prevenir bucles infinitos

    // Asegurar TRIG en bajo
    REG32(GPIO_OUT_W1TC_REG) = TRIG_MASK;
    tiny_delay();

    // Pulso de 10µs aprox en TRIG
    REG32(GPIO_OUT_W1TS_REG) = TRIG_MASK;
    for (volatile uint32_t i = 0; i < 2000; ++i) { // ajuste fino si querés
        __asm__ volatile("nop");
    }
    REG32(GPIO_OUT_W1TC_REG) = TRIG_MASK;

    // Esperar a que ECHO se ponga en alto (inicio pulso)
    while (((REG32(GPIO_IN_REG) & ECHO_MASK) == 0U) && (timeout < HCSR04_TIMEOUT)) {
        timeout++;
    }
    if (timeout >= HCSR04_TIMEOUT) {
        return 0;   // no llegó pulso
    }
 // 🔥 Capturar Tiempo de Inicio (en µs)
    start_time = timer_get_us();

    // Medir cuánto tiempo se mantiene en alto (Timer con Polling)
    timeout = 0;
    while ((REG32(GPIO_IN_REG) & ECHO_MASK) != 0U && (timeout < HCSR04_TIMEOUT)) {
        timeout++; // Incrementamos el timeout para evitar hang
    }
    if (timeout >= HCSR04_TIMEOUT) {
        // Pulso demasiado largo, error o fuera de rango.
    }
    
    // 🔥 Capturar Tiempo de Fin (en µs)
    end_time = timer_get_us();

    // Devolver la duración del pulso en µs
    return (uint32_t)(end_time - start_time);
    /////////////////////////////////////////////////////////////////////
  const uint32_t timeout_us = 30000U; // 30 ms timeout (ajustá si querés)
    uint64_t t0, t1;
    uint64_t deadline;

    // 1) Asegurar TRIG = 0
    REG32(GPIO_OUT_W1TC_REG) = TRIG_MASK;
    // pequeño delay para estabilizar nivel
    for (volatile int i=0; i<200; ++i) __asm__ volatile("nop");

    // 2) Pulso TRIG ~10 us (hacemos ~12-15us para estar tranquilos)
    REG32(GPIO_OUT_W1TS_REG) = TRIG_MASK;
    // delay calibrado: 10-15 us con loop (si querés precision usar timer)
    for (volatile int i=0; i<1200; ++i) __asm__ volatile("nop"); // ajustá si hace falta
    REG32(GPIO_OUT_W1TC_REG) = TRIG_MASK;

    // 3) Esperar subida de ECHO (con timeout usando timer_get_us)
    // Forzar snapshot antes de leer timer
    REG32(TIMG_T0_UPDATE_REG) = TIMG_T0_LOAD_EN;
    t0 = timer_get_us();
    deadline = t0 + timeout_us;

    // Esperar a que ECHO sea HIGH
    while ((REG32(GPIO_IN_REG) & ECHO_MASK) == 0U) {
        REG32(TIMG_T0_UPDATE_REG) = TIMG_T0_LOAD_EN;
        uint64_t now = timer_get_us();
        if (now >= deadline) return 0; // timeout esperando subida
    }

    // 4) Capturar tiempo de inicio (usar snapshot)
    REG32(TIMG_T0_UPDATE_REG) = TIMG_T0_LOAD_EN;
    t0 = timer_get_us();

    // 5) Esperar a que ECHO vuelva a LOW (pulso final)
    while ((REG32(GPIO_IN_REG) & ECHO_MASK) != 0U) {
        REG32(TIMG_T0_UPDATE_REG) = TIMG_T0_LOAD_EN;
        uint64_t now = timer_get_us();
        if (now >= deadline) return 0; // timeout esperando bajada
    }

    // 6) Capturar tiempo de fin
    REG32(TIMG_T0_UPDATE_REG) = TIMG_T0_LOAD_EN;
    t1 = timer_get_us();

    // 7) Retornar diferencia en µs
    if (t1 > t0) return (uint32_t)(t1 - t0);
    return 0;
}*/

 static uint32_t hcsr04_measure_pulse(void) {
    uint32_t count = 0;
    uint32_t timeout = 0;
    // Asegurar TRIG en bajo
    REG32(GPIO_OUT_W1TC_REG) = TRIG_MASK;
    tiny_delay();
    // Pulso de 10µs aprox en TRIG
    REG32(GPIO_OUT_W1TS_REG) = TRIG_MASK;
    for (volatile uint32_t i = 0; i < 2000; ++i) { // ajuste fino si querés
        __asm__ volatile("nop");
    }
    REG32(GPIO_OUT_W1TC_REG) = TRIG_MASK;
    // Esperar a que ECHO se ponga en alto (inicio pulso)
    while (((REG32(GPIO_IN_REG) & ECHO_MASK) == 0U) && (timeout < HCSR04_TIMEOUT)) {
        timeout++;
    }
    if (timeout >= HCSR04_TIMEOUT) {
        return 0;   // no llegó pulso
    }
    // Medir cuánto tiempo se mantiene en alto
    while ((REG32(GPIO_IN_REG) & ECHO_MASK) != 0U) {
        count++;
        if (count >= HCSR04_TIMEOUT) {
            break;  // por seguridad
        }
    }
    return count;
} 

#define TIMG_T0LO_REG         (*(volatile uint32_t*)(TIMG0_BASE + 0x0004))
#define TIMG_T0HI_REG         (*(volatile uint32_t*)(TIMG0_BASE + 0x0008))
#define TIMG_T0UPDATE_REG     (*(volatile uint32_t*)(TIMG0_BASE + 0x000C))

static uint64_t t0_now_us(void)
{
    TIMG_T0UPDATE_REG = (1U << 31);
    while (TIMG_T0UPDATE_REG & (1U << 31)) { }
    uint32_t lo = TIMG_T0LO_REG;
    uint32_t hi = TIMG_T0HI_REG;
    return (((uint64_t)hi) << 32) | lo;
}

int main(void) {
    
    // Deshabilitar watchdogs para bucle infinito didáctico
    disable_timg_wdt(TIMG0_BASE);
    disable_timg_wdt(TIMG1_BASE);
    disable_rtc_wdts();
    
    // Inicializaciones básicas
    gpio_init();
    adc_init();    
    ledc_init();
    uart_init(); 
    timer_init();
    uart_puts("Sistema iniciado. Esperando boton/pulso...\r\n"); // Mensaje de inicio


    // Bucle principal: fade in/out con PWM
    uint32_t duty = 0;
    int8_t step = 1;

    while (1) {
/*
        uint64_t a = t0_now_us(); 
        tiny_delay(); 
        uint64_t b = t0_now_us(); 
        if (b != a) { 
            ledc_set_duty(0); // TIMER ANDA 
        } else { 
            uint32_t sample = adc_sample_once();
            uint32_t pwm_input = (sample > ADC_ZERO_BIAS) ? (sample - ADC_ZERO_BIAS) : 0U;
            uint32_t pwm_range = 4095U - ADC_ZERO_BIAS;
            uint32_t duty = (pwm_input * LEDC_DUTY_MAX) / pwm_range;
            ledc_set_duty(duty); 
            if ((REG32(GPIO_IN_REG)& BUTTON_MASK) != 0U){ 
                ledc_set_duty(LEDC_DUTY_MAX); 
            } 
        } 
        short_delay();


        /*uint32_t pulse = hcsr04_measure_pulse();

        if (pulse == 0) {

            uint32_t sample = adc_sample_once();
            uart_puts("pulso siempre 0.\r\n"); // Mensaje de inicio
            if (sample <= ADC_THRESHOLD){
                ledc_set_duty(duty);
                duty += step;

                if (duty == LEDC_DUTY_MAX || duty == 0) {
                    step = -step; // Cambio de dirección
                }       // sin pulso → LED apagado
            }else{
                uint32_t pwm_input = (sample > ADC_ZERO_BIAS) ? (sample - ADC_ZERO_BIAS) : 0U;
                uint32_t pwm_range = 4095U - ADC_ZERO_BIAS;
                uint32_t duty = (pwm_input * LEDC_DUTY_MAX) / pwm_range;
                ledc_set_duty(duty);
                uart_puts("Algo.\r\n"); // Mensaje de inicio
            }

        } else if (pulse <= HCSR04_NEAR_THRESHOLD) {
            ledc_set_duty(LEDC_DUTY_MAX);     // pulso chico → objeto lejos
        } else {
            ledc_set_duty(0); // pulso grande → objeto cerca
        }
        short_delay();*/
        
        // 🔥 Leer GPIO2 digital
        //uint32_t button = (REG32(GPIO_IN_REG)& BUTTON_MASK) != 0U;

        // Medir pulso del HC-SR04
        uint32_t pulse = hcsr04_measure_pulse(); //Descomentar esta y comentar la de arriba para usar el sensor

            uint16_t sample = adc_sample_once(); 
            uint32_t dynamic_offset = ( (uint32_t)sample * PULSE_RANGE) / ADC_MAX_RANGE;
            uint32_t dynamic_threshold = MIN_PULSE_THRESHOLD + dynamic_offset-ADC_ZERO_BIAS;


         if (pulse < dynamic_threshold) {//>< 
            /*uint16_t sample = adc_sample_once(); 
            uint32_t pwm_input = sample-ADC_THRESHOLD;
            uint32_t pwm_range = 4095U; 
            uint32_t dynamic_threshold = (*pwm_input) / pwm_range;*/
            ledc_set_duty(LEDC_DUTY_MAX);

            // 🔥 NUEVO: Enviar mensaje a la consola
            // \r\n (Carriage Return + New Line) es importante para saltos de línea
            uart_puts("!ATENCION: Deteccion activada. LED detenido.\r\n");

        } else{
            ledc_set_duty(0);
            /*ledc_set_duty(duty);
            duty += step;

            if (duty == LEDC_DUTY_MAX || duty == 0) {
                step = -step; // Cambio de dirección
            }*/
        }
        
        short_delay();
        
    }
}