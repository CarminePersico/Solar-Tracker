/*
 * Solar Tracker Dual-Axis + INA219 + OLED + RTC Wakeup Timer
 * NUCLEO-G474RE (STM32G474RETx) -- HSI 16 MHz, bare-metal C puro
 */

#include <stdint.h>
#include <stdlib.h> /* Aggiunto per abs() del tracking */

/* ================================================================
   BASE ADDRESSES
   ================================================================ */
#define RCC_BASE    0x40021000UL
#define GPIOA_BASE  0x48000000UL
#define GPIOB_BASE  0x48000400UL
#define GPIOC_BASE  0x48000800UL
#define TIM2_BASE   0x40000000UL
#define ADC1_BASE   0x50000000UL
#define ADC2_BASE   0x50000100UL
#define ADC12_BASE  0x50000300UL
#define USART2_BASE 0x40004400UL
#define I2C1_BASE   0x40005400UL
#define EXTI_BASE   0x40010400UL
#define PWR_BASE    0x40007000UL
#define RTC_BASE    0x40002800UL

/* RCC */
#define RCC_AHB2ENR  (*(volatile uint32_t*)(RCC_BASE + 0x4C))
#define RCC_APB2ENR  (*(volatile uint32_t*)(RCC_BASE + 0x60))
#define RCC_APB1ENR1 (*(volatile uint32_t*)(RCC_BASE + 0x58))
#define RCC_BDCR     (*(volatile uint32_t*)(RCC_BASE + 0x90))
#define RCC_CSR      (*(volatile uint32_t*)(RCC_BASE + 0x94))

/* GPIO */
#define GPIOA_MODER  (*(volatile uint32_t*)(GPIOA_BASE + 0x00))
#define GPIOA_AFRL   (*(volatile uint32_t*)(GPIOA_BASE + 0x20))
#define GPIOA_ODR    (*(volatile uint32_t*)(GPIOA_BASE + 0x14))

#define GPIOB_MODER  (*(volatile uint32_t*)(GPIOB_BASE + 0x00))
#define GPIOB_OTYPER (*(volatile uint32_t*)(GPIOB_BASE + 0x04))
#define GPIOB_PUPDR  (*(volatile uint32_t*)(GPIOB_BASE + 0x0C))
#define GPIOB_AFRL   (*(volatile uint32_t*)(GPIOB_BASE + 0x20))
#define GPIOB_AFRH   (*(volatile uint32_t*)(GPIOB_BASE + 0x24))

#define GPIOC_MODER  (*(volatile uint32_t*)(GPIOC_BASE + 0x00))

/* TIM2 */
#define TIM2_CR1   (*(volatile uint32_t*)(TIM2_BASE + 0x00))
#define TIM2_EGR   (*(volatile uint32_t*)(TIM2_BASE + 0x14))
#define TIM2_CCMR1 (*(volatile uint32_t*)(TIM2_BASE + 0x18))
#define TIM2_CCMR2 (*(volatile uint32_t*)(TIM2_BASE + 0x1C))
#define TIM2_CCER  (*(volatile uint32_t*)(TIM2_BASE + 0x20))
#define TIM2_PSC   (*(volatile uint32_t*)(TIM2_BASE + 0x28))
#define TIM2_ARR   (*(volatile uint32_t*)(TIM2_BASE + 0x2C))
#define TIM2_CCR2  (*(volatile uint32_t*)(TIM2_BASE + 0x38))
#define TIM2_CCR3  (*(volatile uint32_t*)(TIM2_BASE + 0x3C))
#define TIM2_DIER  (*(volatile uint32_t*)(TIM2_BASE + 0x0C))
#define TIM2_SR    (*(volatile uint32_t*)(TIM2_BASE + 0x10))

/* ADC */
#define ADC12_CCR  (*(volatile uint32_t*)(ADC12_BASE + 0x08))
#define ADC1_ISR   (*(volatile uint32_t*)(ADC1_BASE  + 0x00))
#define ADC1_CR    (*(volatile uint32_t*)(ADC1_BASE  + 0x08))
#define ADC1_SMPR1 (*(volatile uint32_t*)(ADC1_BASE  + 0x14))
#define ADC1_SMPR2 (*(volatile uint32_t*)(ADC1_BASE  + 0x18))
#define ADC1_SQR1  (*(volatile uint32_t*)(ADC1_BASE  + 0x30))
#define ADC1_DR    (*(volatile uint32_t*)(ADC1_BASE  + 0x40))
#define ADC2_ISR   (*(volatile uint32_t*)(ADC2_BASE  + 0x00))
#define ADC2_CR    (*(volatile uint32_t*)(ADC2_BASE  + 0x08))
#define ADC2_SMPR2 (*(volatile uint32_t*)(ADC2_BASE  + 0x18))
#define ADC2_SQR1  (*(volatile uint32_t*)(ADC2_BASE  + 0x30))
#define ADC2_DR    (*(volatile uint32_t*)(ADC2_BASE  + 0x40))

/* USART2 */
#define USART2_CR1 (*(volatile uint32_t*)(USART2_BASE + 0x00))
#define USART2_BRR (*(volatile uint32_t*)(USART2_BASE + 0x0C))
#define USART2_ISR (*(volatile uint32_t*)(USART2_BASE + 0x1C))
#define USART2_TDR (*(volatile uint32_t*)(USART2_BASE + 0x28))

/* I2C1 */
#define I2C1_CR1     (*(volatile uint32_t*)(I2C1_BASE + 0x00))
#define I2C1_CR2     (*(volatile uint32_t*)(I2C1_BASE + 0x04))
#define I2C1_TIMINGR (*(volatile uint32_t*)(I2C1_BASE + 0x10))
#define I2C1_ISR     (*(volatile uint32_t*)(I2C1_BASE + 0x18))
#define I2C1_ICR     (*(volatile uint32_t*)(I2C1_BASE + 0x1C))
#define I2C1_RXDR    (*(volatile uint32_t*)(I2C1_BASE + 0x24))
#define I2C1_TXDR    (*(volatile uint32_t*)(I2C1_BASE + 0x28))

#define I2C_ISR_TXIS   (1u << 1)
#define I2C_ISR_RXNE   (1u << 2)
#define I2C_ISR_TC     (1u << 6)
#define I2C_ISR_BUSY   (1u << 15)
#define I2C_ICR_STOPCF (1u << 5)
#define I2C_TIMEOUT    50000u

/* EXTI */
#define EXTI_RTSR1   (*(volatile uint32_t*)(EXTI_BASE + 0x00))
#define EXTI_FTSR1   (*(volatile uint32_t*)(EXTI_BASE + 0x04))
#define EXTI_SWIER1  (*(volatile uint32_t*)(EXTI_BASE + 0x08))
#define EXTI_RPR1    (*(volatile uint32_t*)(EXTI_BASE + 0x0C))
#define EXTI_FPR1    (*(volatile uint32_t*)(EXTI_BASE + 0x10))
#define EXTI_EXTICR4 (*(volatile uint32_t*)(EXTI_BASE + 0x6C))
#define EXTI_IMR1    (*(volatile uint32_t*)(EXTI_BASE + 0x80))
#define EXTI_EMR1    (*(volatile uint32_t*)(0x40010484u))

/* NVIC */
#define NVIC_ISER0 (*(volatile uint32_t*)0xE000E100)
#define NVIC_ISER1 (*(volatile uint32_t*)0xE000E104)
#define NVIC_ICPR0 (*(volatile uint32_t*)0xE000E280)

/* PWR / SCB */
#define PWR_CR1 (*(volatile uint32_t*)(PWR_BASE + 0x00))
#define SCB_SCR (*(volatile uint32_t*)0xE000ED10)

/* RTC */
#define RTC_TR   (*(volatile uint32_t*)(RTC_BASE + 0x00))
#define RTC_DR   (*(volatile uint32_t*)(RTC_BASE + 0x04))
#define RTC_ICSR (*(volatile uint32_t*)(RTC_BASE + 0x0C))
#define RTC_PRER (*(volatile uint32_t*)(RTC_BASE + 0x10))
#define RTC_WUTR (*(volatile uint32_t*)(RTC_BASE + 0x14))
#define RTC_CR   (*(volatile uint32_t*)(RTC_BASE + 0x18))
#define RTC_WPR  (*(volatile uint32_t*)(RTC_BASE + 0x24))
#define RTC_SR   (*(volatile uint32_t*)(RTC_BASE + 0x50))
#define RTC_SCR  (*(volatile uint32_t*)(RTC_BASE + 0x5C))

#define RTC_CR_WUTE  (1u << 10)
#define RTC_CR_WUTIE (1u << 14)
#define RTC_ICSR_WUTWF (1u << 2)
#define RTC_ICSR_INITF (1u << 6)
#define RTC_ICSR_INIT  (1u << 7)
#define RTC_SCR_CWUTF  (1u << 2)

/* ================================================================
   INA219 (Aggiornato)
   ================================================================ */
#define INA219_ADDR    0x40u
#define INA219_REG_CFG 0x00u
#define INA219_REG_SHV 0x01u
#define INA219_REG_BUS 0x02u
#define INA219_REG_CUR 0x04u
#define INA219_REG_CAL 0x05u
#define INA219_CFG_VAL 0x399Fu
#define INA219_CAL_VAL 4096u /* LSB = 0.1 mA */

/* ================================================================
   TRACKING (Aggiornato dal Codice 1)
   ================================================================ */
#define TOLERANCE 350
#define STEP      11
#define PWM_MIN   611
#define PWM_MAX   2500

/* ================================================================
   FASI OPERATIVE
   ================================================================ */
#define PHASE_DEMO        0u
#define PHASE_INDUSTRIAL  1u
#define DEMO_CYCLES       20u
#define WAKEUP_DEMO_S     2u
#define WAKEUP_IND_S      900u

/* ================================================================
   VARIABILI GLOBALI
   ================================================================ */
volatile uint8_t rtc_wakeup_flag = 0;
volatile uint8_t b1_flag         = 0;

/* Posizioni iniziali */
static int32_t angle_pan  = 1633;
static int32_t angle_tilt = 1833;

static uint8_t  current_phase    = PHASE_DEMO;
static uint32_t demo_cycle_count = 0;
static uint32_t total_cycles     = 0;

static int32_t oled_vbat = 0;
static int32_t oled_curr = 0;
static int32_t oled_pwr  = 0;
static uint8_t oled_online = 0;

/* ================================================================
   UTILITY
   ================================================================ */
static void delay_ms(volatile uint32_t ms) {
    while (ms--) { volatile uint32_t i = 3400; while (i--); }
}

static void uart_putc(char c) {
    while (!(USART2_ISR & (1u << 7)));
    USART2_TDR = (uint32_t)c;
}
static void uart_puts(const char *s) { while (*s) uart_putc(*s++); }
static void uart_puti(int32_t n) {
    char buf[12]; int i = 0;
    if (n < 0) { uart_putc('-'); n = -n; }
    if (n == 0) { uart_putc('0'); return; }
    while (n > 0) { buf[i++] = '0' + (char)(n % 10); n /= 10; }
    while (i--) uart_putc(buf[i]);
}

static void uart_flush(void) {
    while (!(USART2_ISR & (1u << 6)));
    delay_ms(2);
}

/* ================================================================
   ADC
   ================================================================ */
static void adc1_init(void) {
    ADC1_CR  = 0;
    ADC1_CR  = (1u << 28);
    delay_ms(5);
    ADC1_CR |= (1u << 31);
    while (ADC1_CR & (1u << 31));
    ADC1_ISR |= (1u << 0);
    ADC1_CR  |= (1u << 0);
    while (!(ADC1_ISR & (1u << 0)));
    ADC1_SMPR1 = (5u << 6) | (5u << 21);
    ADC1_SMPR2 = (5u << 15);
}

static void adc2_init(void) {
    ADC2_CR  = 0;
    ADC2_CR  = (1u << 28);
    delay_ms(5);
    ADC2_CR |= (1u << 31);
    while (ADC2_CR & (1u << 31));
    ADC2_ISR |= (1u << 0);
    ADC2_CR  |= (1u << 0);
    while (!(ADC2_ISR & (1u << 0)));
    ADC2_SMPR2 = (5u << 21);
}

static void adc_wakeup_restore(void) {
    if (!(ADC1_CR & (1u << 0))) { adc1_init(); }
    if (!(ADC2_CR & (1u << 0))) { adc2_init(); }
}

static uint32_t adc1_read(uint32_t ch) {
    ADC1_SQR1 = (ch << 6);
    ADC1_CR  |= (1u << 2);
    while (!(ADC1_ISR & (1u << 2)));
    uint32_t v = ADC1_DR;
    ADC1_ISR  = (1u << 2);
    return v;
}
static uint32_t adc2_read(uint32_t ch) {
    ADC2_SQR1 = (ch << 6);
    ADC2_CR  |= (1u << 2);
    while (!(ADC2_ISR & (1u << 2)));
    uint32_t v = ADC2_DR;
    ADC2_ISR  = (1u << 2);
    return v;
}

/* ================================================================
   I2C1 / INA219 (Aggiornato per supportare i decimali della corrente)
   ================================================================ */
static int i2c_write_reg(uint8_t addr7, uint8_t reg, uint16_t val) {
    volatile uint32_t t = I2C_TIMEOUT;
    while ((I2C1_ISR & I2C_ISR_BUSY) && --t); if (!t) return -1;
    I2C1_CR2 = ((uint32_t)(addr7<<1) & 0x3FFu) | (3u<<16) | (1u<<25) | (1u<<13);
    t = I2C_TIMEOUT;
    while (!(I2C1_ISR & I2C_ISR_TXIS) && --t); if (!t) return -2;
    I2C1_TXDR = reg;
    t = I2C_TIMEOUT;
    while (!(I2C1_ISR & I2C_ISR_TXIS) && --t); if (!t) return -3;
    I2C1_TXDR = (val >> 8) & 0xFFu;
    t = I2C_TIMEOUT;
    while (!(I2C1_ISR & I2C_ISR_TXIS) && --t); if (!t) return -4;
    I2C1_TXDR = val & 0xFFu;
    t = I2C_TIMEOUT;
    while (!(I2C1_ISR & (1u<<5)) && --t);
    I2C1_ICR = I2C_ICR_STOPCF;
    return 0;
}

static int i2c_read_reg16(uint8_t addr7, uint8_t reg, int16_t *out) {
    volatile uint32_t t = I2C_TIMEOUT;
    while ((I2C1_ISR & I2C_ISR_BUSY) && --t); if (!t) return -1;
    I2C1_CR2 = ((uint32_t)(addr7<<1) & 0x3FFu) | (1u<<16) | (1u<<13);
    t = I2C_TIMEOUT;
    while (!(I2C1_ISR & I2C_ISR_TXIS) && --t); if (!t) return -2;
    I2C1_TXDR = reg;
    t = I2C_TIMEOUT;
    while (!(I2C1_ISR & I2C_ISR_TC) && --t); if (!t) return -3;
    I2C1_CR2 = ((uint32_t)(addr7<<1) & 0x3FFu) | (1u<<10) | (2u<<16) | (1u<<25) | (1u<<13);
    t = I2C_TIMEOUT;
    while (!(I2C1_ISR & I2C_ISR_RXNE) && --t); if (!t) return -4;
    uint8_t msb = (uint8_t)I2C1_RXDR;
    t = I2C_TIMEOUT;
    while (!(I2C1_ISR & I2C_ISR_RXNE) && --t); if (!t) return -5;
    uint8_t lsb = (uint8_t)I2C1_RXDR;
    t = I2C_TIMEOUT;
    while (!(I2C1_ISR & (1u<<5)) && --t);
    I2C1_ICR = I2C_ICR_STOPCF;
    *out = (int16_t)((uint16_t)(msb<<8) | lsb);
    return 0;
}

static void ina219_print(void) {
    int16_t sv_raw, bv_raw, cv_raw;
    int r = 0;

    r |= i2c_read_reg16(INA219_ADDR, INA219_REG_SHV, &sv_raw);
    r |= i2c_read_reg16(INA219_ADDR, INA219_REG_BUS, &bv_raw);
    r |= i2c_read_reg16(INA219_ADDR, INA219_REG_CUR, &cv_raw);

    if (r != 0) {
        uart_puts(" | INA_ERR");
        oled_vbat=0; oled_curr=0; oled_pwr=0;
        return;
    }

    int32_t millivolt = (bv_raw >> 3) * 4;
    int32_t current_decima = (int32_t)cv_raw;
    int32_t milliwatt = (millivolt * current_decima) / 10000;

    // Calcolo Percentuale Batteria Li-Ion (Range 3200mV - 4200mV)
    /* Calcolo Percentuale (Range 3300mV - 4100mV) */
        int32_t pct = ((millivolt - 3300) * 100) / 800;
        if (pct > 100) pct = 100;
        if (pct < 0) pct = 0;

    oled_vbat = millivolt;
    oled_curr = current_decima / 10;
    oled_pwr  = milliwatt;

    // Nuova stampa UART con percentuale
    uart_puts(" | BATT: "); uart_puti(millivolt);
    uart_puts(" mV ("); uart_puti(pct); uart_puts("%) | CURR: ");

    int32_t c_temp = current_decima;
    if (c_temp < 0) { uart_putc('-'); c_temp = -c_temp; }
    uart_puti(c_temp / 10); uart_putc('.'); uart_puti(c_temp % 10);
    uart_puts(" mA | PWR: "); uart_puti(milliwatt); uart_puts(" mW");
}

/* ================================================================
   OLED SSD1306 (I2C1)
   ================================================================ */
#define OLED_ADDR 0x3C

static int i2c_write_oled(uint8_t ctrl, uint8_t data) {
    volatile uint32_t t = I2C_TIMEOUT;
    while ((I2C1_ISR & I2C_ISR_BUSY) && --t);
    if (!t) return -1;
    I2C1_CR2 = ((uint32_t)(OLED_ADDR<<1) & 0x3FFu) | (2u<<16) | (1u<<25) | (1u<<13);
    t = I2C_TIMEOUT;
    while (!(I2C1_ISR & I2C_ISR_TXIS) && --t);
    if (!t) return -2;
    I2C1_TXDR = ctrl;
    t = I2C_TIMEOUT;
    while (!(I2C1_ISR & I2C_ISR_TXIS) && --t);
    if (!t) return -3;
    I2C1_TXDR = data;
    t = I2C_TIMEOUT;
    while (!(I2C1_ISR & (1u<<5)) && --t);
    I2C1_ICR = I2C_ICR_STOPCF;
    return 0;
}

static int oled_cmd(uint8_t cmd)   { return i2c_write_oled(0x00, cmd); }
static int oled_data(uint8_t data) { return i2c_write_oled(0x40, data); }

static void oled_wake(void)  { oled_cmd(0xAF); }
static void oled_sleep(void) { oled_cmd(0xAE); }

/* ================================================================
   SERVO
   ================================================================ */
static void servo_enable(void) {
    TIM2_CCR2 = (uint32_t)angle_pan;
    TIM2_CCR3 = (uint32_t)angle_tilt;
    TIM2_EGR  |= (1u << 0);
    TIM2_CR1  |= (1u << 0);
    TIM2_CCER |= (1u << 4) | (1u << 8);
}

static void servo_disable(void) {
    TIM2_CCER &= ~((1u << 4) | (1u << 8));
}

/* ================================================================
   TRACKING (Aggiornato con la logica esatta del Codice 1)
   ================================================================ */
static void do_tracking(void) {
    uint32_t passi = 0;
    uint32_t lt = 0, rt = 0, ld = 0, rd = 0;
    int dvert = 0, dhoriz = 0;

    // Ciclo "Burst": continua a leggere e muovere finché non è allineato
        while (passi < 50) {
            /*

    MAPPATURA DI EMERGENZA */
      // Prima leggiamo tutti i pin grezzi
      uint32_t pin_A4 = adc1_read(7);  // Il vecchio TL
      uint32_t pin_A1 = adc1_read(2);  // Il vecchio TR
      uint32_t pin_A3 = adc1_read(15); // Il vecchio BL
      uint32_t pin_A2 = adc2_read(17); // Il vecchio BR

            // Ora assegniamo forzatamente i valori corretti in base a quello che mi hai detto:
            // Tu hai detto: TL = TR (il pin_A2 risponde al TL)
            lt = pin_A4;

            // Tu hai detto: TR = BR (il pin_A4 risponde al TR)
            rt = pin_A1;

            // Tu hai detto: BL = BL (corretto)
            ld = pin_A3;

            // Tu hai detto: BR = TR (il pin_A1 risponde al BR)
            rd = pin_A2;

            int avt = (lt + rt) / 2;
            int avd = (ld + rd) / 2;
            int avl = (lt + ld) / 2;
            int avr = (rt + rd) / 2;

        dvert  = abs(avt - avd);
        dhoriz = abs(avl - avr);

        uint8_t motore_mosso = 0;

        /* Gestione PAN (SEGNI INVERTITI per inseguire la luce dal lato giusto) */
        if (dhoriz > TOLERANCE) {
            if (avl > avr) {
                /* C'è più LUCE a SINISTRA. Andiamo a Sinistra! (segno -) */
                if (angle_pan - STEP >= PWM_MIN) { angle_pan -= STEP; motore_mosso = 1; }
            } else if (avl < avr) {
                /* C'è più LUCE a DESTRA. Andiamo a Destra! (segno +) */
                if (angle_pan + STEP <= PWM_MAX) { angle_pan += STEP; motore_mosso = 1; }
            }
            TIM2_CCR2 = (uint32_t)angle_pan;
        }

        /* Gestione TILT (SEGNI INVERTITI per coerenza) */
        if (dvert > TOLERANCE) {
            if (avt > avd) {
                /* C'è più LUCE SOPRA. Andiamo in Alto! (segno -) */
                if (angle_tilt - STEP >= PWM_MIN) { angle_tilt -= STEP; motore_mosso = 1; }
            } else if (avt < avd) {
                /* C'è più LUCE SOTTO. Andiamo in Basso! (segno +) */
                if (angle_tilt + STEP <= PWM_MAX) { angle_tilt += STEP; motore_mosso = 1; }
            }
            TIM2_CCR3 = (uint32_t)angle_tilt;
        }

        if (motore_mosso == 0) {
            break;
        }

        delay_ms(20);
        passi++;
    }

    /* Stampe di Debug */
    uart_puts(" | LDR: TL="); uart_puti(lt);
    uart_puts(" TR=");        uart_puti(rt);
    uart_puts(" BL=");        uart_puti(ld);
    uart_puts(" BR=");        uart_puti(rd);

    uart_puts(" | Diff: H="); uart_puti(dhoriz);
    uart_puts(" V=");         uart_puti(dvert);

    uart_puts(" | PASSI=");   uart_puti(passi);
    uart_puts(" | PAN=");     uart_puti(angle_pan);
    uart_puts(" TILT=");      uart_puti(angle_tilt);
}
/* ================================================================
   RTC WAKEUP TIMER E SLEEP
   ================================================================ */
static void rtc_set_wakeup_period(uint32_t seconds) {
    volatile uint32_t t;
    RTC_WPR = 0xCAu;
    RTC_WPR = 0x53u;

    RTC_CR &= ~RTC_CR_WUTE;
    t = 200000u;
    while (!(RTC_ICSR & RTC_ICSR_WUTWF) && --t);

    uint32_t wut = seconds - 1u;
    RTC_WUTR = wut;

    RTC_CR |= RTC_CR_WUTE;
    RTC_WPR = 0xFFu;
}

static void rtc_init(void) {
    volatile uint32_t t;
    PWR_CR1 |= (1u << 8);
    RCC_CSR |= (1u << 0);
    t = 300000u;
    while (!(RCC_CSR & (1u << 1)) && --t);

    RCC_BDCR |= (1u << 16);
    RCC_BDCR &= ~(1u << 16);
    delay_ms(2);
    RCC_BDCR |= (2u << 8);
    RCC_BDCR |= (1u << 15);
    delay_ms(5);

    RTC_WPR = 0xCAu;
    RTC_WPR = 0x53u;

    RTC_ICSR |= RTC_ICSR_INIT;
    t = 300000u;
    while (!(RTC_ICSR & RTC_ICSR_INITF) && --t);

    RTC_PRER = (127u << 16) | 249u;
    RTC_ICSR &= ~RTC_ICSR_INIT;

    RTC_CR &= ~RTC_CR_WUTE;
    t = 200000u;
    while (!(RTC_ICSR & RTC_ICSR_WUTWF) && --t);

    uint32_t wut = WAKEUP_DEMO_S - 1u;
    RTC_WUTR = wut;
    RTC_CR   &= ~(7u << 0);
    RTC_CR   |=  (4u << 0);
    RTC_CR   |=  RTC_CR_WUTIE;
    RTC_CR   |=  RTC_CR_WUTE;

    RTC_WPR = 0xFFu;
    RTC_SCR = RTC_SCR_CWUTF;

    EXTI_EMR1  |= (1u << 20);
}

void HardFault_Handler(void) {
    while(1) {
        GPIOA_ODR ^= (1u << 5);
        volatile uint32_t i = 80000; while(i--);
    }
}

void TIM2_IRQHandler(void) {
    TIM2_SR &= ~(1u << 0);
}

void RTC_WKUP_IRQHandler(void) {
    GPIOA_ODR ^= (1u << 5);
    RTC_WPR = 0xCAu; RTC_WPR = 0x53u;
    RTC_CR  &= ~RTC_CR_WUTE;
    volatile uint32_t t = 100000u;
    while (!(RTC_ICSR & RTC_ICSR_WUTWF) && --t);
    RTC_SCR  = RTC_SCR_CWUTF;
    RTC_CR  |=  RTC_CR_WUTE;
    RTC_WPR  = 0xFFu;
    EXTI_RPR1 = (1u << 20);
    rtc_wakeup_flag = 1;
}

void EXTI15_10_IRQHandler(void) {
    if (EXTI_FPR1 & (1u << 13)) {
        EXTI_FPR1 = (1u << 13);
        b1_flag = 1;
    }
}

/* ============================================================
 * LIBRERIA OLED (Contenuta)
 * ============================================================ */
static const uint8_t font8x16[44][16] = {
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x00,0x18,0x18,0x7E,0x7E,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00,0x00},
{0x00,0x00,0x00,0x0C,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00,0x00,0x00},
{0x00,0x06,0x09,0x09,0x09,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0xFE,0x01,0x01,0x01,0x01,0x01,0xFE,0x00,0x3F,0x40,0x40,0x40,0x40,0x40,0x3F,0x00},
{0x00,0x04,0x02,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x00},
{0x02,0x01,0x81,0x41,0x21,0x11,0x0E,0x00,0x7E,0x41,0x40,0x40,0x40,0x40,0x40,0x00},
{0x02,0x01,0x01,0x11,0x11,0x11,0xEE,0x00,0x30,0x40,0x40,0x40,0x40,0x40,0x3F,0x00},
{0x30,0x28,0x24,0x22,0xFF,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00,0x00,0x00},
{0x1F,0x11,0x11,0x11,0x11,0x11,0xE0,0x00,0x30,0x40,0x40,0x40,0x40,0x40,0x3F,0x00},
{0xFE,0x11,0x11,0x11,0x11,0x11,0xE0,0x00,0x3F,0x40,0x40,0x40,0x40,0x40,0x3F,0x00},
{0x01,0x01,0x01,0x81,0x61,0x19,0x07,0x00,0x60,0x18,0x06,0x01,0x00,0x00,0x00,0x00},
{0xEE,0x11,0x11,0x11,0x11,0x11,0xEE,0x00,0x3F,0x40,0x40,0x40,0x40,0x40,0x3F,0x00},
{0x0E,0x11,0x11,0x11,0x11,0x11,0xFE,0x00,0x30,0x40,0x40,0x40,0x40,0x40,0x3F,0x00},
{0xF8,0x26,0x21,0x21,0x26,0xF8,0x00,0x00,0x7F,0x00,0x00,0x00,0x00,0x7F,0x00,0x00},
{0xFF,0x11,0x11,0x11,0x11,0x0E,0xE0,0x00,0x7F,0x40,0x40,0x40,0x40,0x40,0x3F,0x00},
{0xFE,0x01,0x01,0x01,0x01,0x01,0x02,0x00,0x3F,0x40,0x40,0x40,0x40,0x40,0x00,0x00},
{0xFF,0x01,0x01,0x01,0x01,0xFE,0x00,0x00,0x7F,0x40,0x40,0x40,0x40,0x3F,0x00,0x00},
{0xFF,0x11,0x11,0x11,0x11,0x01,0x01,0x00,0x7F,0x40,0x40,0x40,0x40,0x40,0x40,0x00},
{0xFF,0x11,0x11,0x11,0x11,0x01,0x01,0x00,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0xFE,0x01,0x01,0x01,0x41,0x41,0xC2,0x00,0x3F,0x40,0x40,0x40,0x40,0x40,0x3F,0x00},
{0xFF,0x80,0x80,0x80,0x80,0x80,0xFF,0x00,0x7F,0x01,0x01,0x01,0x01,0x01,0x7F,0x00},
{0x01,0x01,0x01,0xFF,0x01,0x01,0x01,0x00,0x40,0x40,0x40,0x7F,0x40,0x40,0x40,0x00},
{0x00,0x00,0x00,0x00,0x01,0x01,0xFF,0x00,0x30,0x40,0x40,0x40,0x40,0x40,0x3F,0x00},
{0xFF,0x20,0x50,0x88,0x04,0x02,0x01,0x00,0x7F,0x00,0x00,0x00,0x01,0x02,0x7C,0x00},
{0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x40,0x40,0x40,0x40,0x40,0x40,0x00},
{0xFF,0x06,0x18,0x60,0x18,0x06,0xFF,0x00,0x7F,0x00,0x00,0x00,0x00,0x00,0x7F,0x00},
{0xFF,0x02,0x04,0x08,0x10,0x20,0xFF,0x00,0x7F,0x00,0x00,0x00,0x00,0x00,0x7F,0x00},
{0xFE,0x01,0x01,0x01,0x01,0x01,0xFE,0x00,0x3F,0x40,0x40,0x40,0x40,0x40,0x3F,0x00},
{0xFF,0x41,0x41,0x41,0x41,0x41,0x3E,0x00,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0xFE,0x01,0x01,0x01,0x01,0x01,0xFE,0x00,0x3F,0x40,0x40,0x40,0x40,0x60,0x7F,0x00},
{0xFF,0x41,0x41,0x41,0xC1,0x41,0x3E,0x00,0x7F,0x00,0x00,0x00,0x01,0x06,0x78,0x00},
{0x0E,0x11,0x11,0x11,0x11,0x11,0xE2,0x00,0x30,0x40,0x40,0x40,0x40,0x40,0x3F,0x00},
{0x01,0x01,0x01,0xFF,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x00},
{0xFF,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x3F,0x40,0x40,0x40,0x40,0x40,0x3F,0x00},
{0x3F,0xC0,0x00,0x00,0x00,0xC0,0x3F,0x00,0x00,0x00,0x03,0x7C,0x03,0x00,0x00,0x00},
{0xFF,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x7F,0x60,0x18,0x07,0x18,0x60,0x7F,0x00},
{0x03,0x0C,0x30,0xC0,0x30,0x0C,0x03,0x00,0x70,0x0C,0x03,0x00,0x03,0x0C,0x70,0x00},
{0x03,0x0C,0x30,0xC0,0x30,0x0C,0x03,0x00,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x00},
{0x81,0x41,0x21,0x11,0x09,0x05,0x03,0x00,0x41,0x42,0x44,0x48,0x50,0x60,0x40,0x00},
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x01,0x01,0x7F,0x01,0x01,0x7F,0x00}, /* 42: m */
{0x06,0x09,0x09,0x06,0xC0,0x30,0x0C,0x00,0x00,0x30,0x0C,0x03,0x30,0x48,0x48,0x30}  /* 43: % */
};

static uint8_t _fidx(char c) {
    if (c == ' ') return 0u;
    if (c == '+') return 1u;
    if (c == '-') return 2u;
    if (c == '.') return 3u;
    if (c == ':') return 4u;
    if ((uint8_t)c == 0xB0u) return 5u;
    if (c == '%') return 43u; /* Aggiunta mappatura simbolo percentuale */
    if (c >= '0' && c <= '9') return 6u + (uint8_t)(c - '0');
    if (c >= 'A' && c <= 'Z') return 16u + (uint8_t)(c - 'A');
    if (c == 'm') return 42u;
    return 0u;
}

static int oled_init(void) {
    if (oled_cmd(0xAE) != 0) return -1;
    oled_cmd(0xD5); oled_cmd(0x80);
    oled_cmd(0xA8); oled_cmd(0x3F);
    oled_cmd(0xD3); oled_cmd(0x00);
    oled_cmd(0x40);
    oled_cmd(0x8D); oled_cmd(0x14);
    oled_cmd(0x20); oled_cmd(0x00);
    oled_cmd(0xA1);
    oled_cmd(0xC8);
    oled_cmd(0xDA); oled_cmd(0x12);
    oled_cmd(0x81); oled_cmd(0xCF);
    oled_cmd(0xD9); oled_cmd(0xF1);
    oled_cmd(0xDB); oled_cmd(0x40);
    oled_cmd(0xA4);
    oled_cmd(0xA6);
    oled_cmd(0xAF);
    return 0;
}

static void oled_clear(void) {
    oled_cmd(0x21); oled_cmd(0u); oled_cmd(127u);
    oled_cmd(0x22); oled_cmd(0u); oled_cmd(7u);
    for (uint16_t i = 0u; i < 1024u; i++) oled_data(0x00u);
}

static void oled_write_char_at(uint8_t page, uint8_t col, char c) {
    const uint8_t *g = font8x16[_fidx(c)];
    oled_cmd(0x21u); oled_cmd(col);  oled_cmd((uint8_t)(col + 7u));
    oled_cmd(0x22u); oled_cmd(page); oled_cmd((uint8_t)(page + 1u));
    for (uint8_t i = 0u; i < 16u; i++) oled_data(g[i]);
}

static void oled_puts(uint8_t page, uint8_t *pcol, const char *s) {
    while (*s && *pcol <= 120u) {
        oled_write_char_at(page, *pcol, *s++);
        *pcol += 8u;
    }
}

static void oled_write_int(uint8_t page, uint8_t *pcol, int32_t val, uint8_t min_w, uint8_t show_sign) {
    char buf[11];
    uint8_t len = 0u;
    uint32_t uval;

    if (val < 0) {
        oled_write_char_at(page, *pcol, '-');
        *pcol += 8u;
        uval = (val == INT32_MIN) ? 2147483648u : (uint32_t)(-val);
    } else {
        if (show_sign) { oled_write_char_at(page, *pcol, '+'); *pcol += 8u; }
        uval = (uint32_t)val;
    }

    do {
        buf[len++] = (char)('0' + uval % 10u);
        uval /= 10u;
    } while (uval);

    while (len < min_w) buf[len++] = '0';

    while (len--) {
        oled_write_char_at(page, *pcol, buf[len]);
        *pcol += 8u;
    }
}

void oled_print_telemetry(int32_t mv, int32_t ma, int32_t mw, int32_t pan, int32_t tilt) {
    uint8_t col;
    oled_clear();

    /* BATT: X.XXV YYP (Massimo 15 caratteri per stare nei 128 pixel dell'OLED) */
    {
        int32_t v_abs  = (mv < 0) ? -mv : mv;
        int32_t v_int  = v_abs / 1000;
        int32_t v_frac = (v_abs % 1000) / 10;

        /* Calcolo Percentuale (Range 3300mV - 4100mV) */
            int32_t pct = ((mv - 3300) * 100) / 800;
            if (pct > 100) pct = 100;
            if (pct < 0) pct = 0;

        col = 0u;
        oled_puts(0u, &col, "BAT: "); /* 5 caratteri */
        if (mv < 0) { oled_write_char_at(0u, col, '-'); col += 8u; }
        oled_write_int(0u, &col, v_int,  1u, 0u);
        oled_write_char_at(0u, col,  '.'); col += 8u;
        oled_write_int(0u, &col, v_frac, 2u, 0u);
        oled_write_char_at(0u, col,  'V'); col += 8u;

        oled_write_char_at(0u, col,  ' '); col += 8u; /* Spazio separatore */

        oled_write_int(0u, &col, pct, 1u, 0u);        /* Numero percentuale */
        oled_write_char_at(0u, col,  '%'); col += 8u;
    }

    /* CURR: +/-XXX mA */
    {
        col = 0u;
        oled_puts(2u, &col, "CURR: ");
        oled_write_int(2u, &col, ma, 1u, 1u);
        oled_write_char_at(2u, col,  ' '); col += 8u;
        oled_write_char_at(2u, col,  'm'); col += 8u;
        oled_write_char_at(2u, col,  'A');
    }

    /* PWR: XXXX mW */
    {
        col = 0u;
        oled_puts(4u, &col, "PWR:  ");
        oled_write_int(4u, &col, mw, 1u, 0u);
        oled_write_char_at(4u, col,  ' '); col += 8u;
        oled_write_char_at(4u, col,  'm'); col += 8u;
        oled_write_char_at(4u, col,  'W');
    }

    /* P:XX° T:XX° */
    {
        col = 0u;
        oled_write_char_at(6u, col,  'P'); col += 8u;
        oled_write_char_at(6u, col,  ':'); col += 8u;
        oled_write_int(6u, &col, pan,  1u, 0u);
        oled_write_char_at(6u, col,  '\xB0'); col += 8u;
        oled_write_char_at(6u, col,  ' '); col += 8u;
        oled_write_char_at(6u, col,  'T'); col += 8u;
        oled_write_char_at(6u, col,  ':'); col += 8u;
        oled_write_int(6u, &col, tilt, 1u, 0u);
        oled_write_char_at(6u, col,  '\xB0');
    }
}

/* ================================================================
   MAIN
   ================================================================ */
int main(void) {
    *(volatile uint32_t*)0x40021060u |= (1u << 0);
    (*(volatile uint32_t*)0xE0042004) &= ~(5u);

    RCC_CSR |= (1u << 23);

    /* 1. Clock enable */
    RCC_AHB2ENR  |= (1u<<0)|(1u<<1)|(1u<<2)|(1u<<13);
    RCC_APB1ENR1 |= (1u<<0) | (1u<<17) | (1u<<21) | (1u<<28);
    RCC_APB2ENR  |= (1u << 0);

    /* 2. GPIO */
    GPIOA_MODER |= (3u << 2);
    GPIOA_MODER |= (3u << 8);
    GPIOA_MODER &= ~(3u<<4);   GPIOA_MODER |= (2u<<4);
    GPIOA_AFRL  &= ~(0xFu<<8); GPIOA_AFRL  |= (7u<<8);

    GPIOB_MODER |= (3u << 0);
    GPIOB_MODER &= ~(3u<<6);    GPIOB_MODER |= (2u<<6);
    GPIOB_AFRL  &= ~(0xFu<<12); GPIOB_AFRL  |= (1u<<12);
    GPIOB_MODER  &= ~(3u<<16);  GPIOB_MODER  |= (2u<<16);
    GPIOB_OTYPER |=  (1u<<8);
    GPIOB_PUPDR  &= ~(3u<<16);  GPIOB_PUPDR  |= (1u<<16);
    GPIOB_AFRH   &= ~(0xFu<<0); GPIOB_AFRH   |= (4u<<0);
    GPIOB_MODER  &= ~(3u<<18);  GPIOB_MODER  |= (2u<<18);
    GPIOB_OTYPER |=  (1u<<9);
    GPIOB_PUPDR  &= ~(3u<<18);  GPIOB_PUPDR  |= (1u<<18);
    GPIOB_AFRH   &= ~(0xFu<<4); GPIOB_AFRH   |= (4u<<4);
    GPIOB_MODER &= ~(3u<<20);   GPIOB_MODER |= (2u<<20);
    GPIOB_AFRH  &= ~(0xFu<<8);  GPIOB_AFRH  |= (1u<<8);

    GPIOC_MODER |= (3u << 2);

    GPIOA_MODER &= ~(3u << 10);
    GPIOA_MODER |=  (1u << 10);
    GPIOA_ODR   |=  (1u << 5);

    /* 3. USART2 */
    USART2_BRR = 139u;
    USART2_CR1 = (1u<<3) | (1u<<0);

    /* 4. ADC */
    ADC12_CCR &= ~(3u << 16);
    ADC12_CCR |=  (1u << 16);
    adc1_init();
    adc2_init();

    /* 5. TIM2 PWM */
    TIM2_PSC = 15u;
    TIM2_ARR = 19999u;
    TIM2_CCMR1 &= ~(0xFFu << 8); TIM2_CCMR1 |= (0x68u << 8);
    TIM2_CCMR2 &= ~(0xFFu);      TIM2_CCMR2 |= (0x68u);
    TIM2_CCR2 = (uint32_t)angle_pan;
    TIM2_CCR3 = (uint32_t)angle_tilt;
    TIM2_EGR  |= (1u << 0);
    TIM2_CR1  |= (1u << 0);
    TIM2_DIER |= (1u << 0);
    *(volatile uint32_t*)0xE000E100u |= (1u << 28);

    /* 6. I2C1 */
    I2C1_CR1     = 0;
    I2C1_TIMINGR = 0x00303D5Bu;
    I2C1_CR1     = (1u << 0);
    delay_ms(2);

    if (oled_init() == 0) {
        oled_online = 1;
        oled_clear();
        uart_puts("OLED: ONLINE\r\n");
    } else {
        uart_puts("[WARN] OLED assente\r\n");
    }

    /* 7. EXTI B1 */
    EXTI_EXTICR4 &= ~(0xFu << 4); EXTI_EXTICR4 |= (0x2u << 4);
    EXTI_FTSR1   |= (1u << 13);
    EXTI_IMR1    |= (1u << 13);
    NVIC_ISER1    = (1u << 8);

    /* 8. Interrupt globali on */
    __asm volatile ("cpsie i" ::: "memory");

    /* 9. INA219 init */
    uart_puts("\r\n=== Solar Tracker | INA219 | OLED ===\r\n");
    if (i2c_write_reg(INA219_ADDR, INA219_REG_CFG, INA219_CFG_VAL) != 0 ||
        i2c_write_reg(INA219_ADDR, INA219_REG_CAL, INA219_CAL_VAL) != 0) {
        uart_puts("[WARN] INA219 non risponde\r\n");
    } else {
        uart_puts("INA219 OK\r\n");
    }

    /* 11. RTC wakeup timer init */
    rtc_init();
    uart_puts("RTC OK -- Fase DEMO attiva\r\n\r\n");

    /* 12. MAIN LOOP */
    while (1) {
        GPIOA_ODR |=  (1u << 5);
        delay_ms(500);
        GPIOA_ODR &= ~(1u << 5);

        total_cycles++;
        rtc_wakeup_flag = 0;
        b1_flag         = 0;

        uart_puts("[C#"); uart_puti((int32_t)total_cycles);
        if (current_phase == PHASE_DEMO) uart_puts(" DEMO]  ");
        else                              uart_puts(" IND]   ");

        adc_wakeup_restore();

        /* Diamo 5 millisecondi all'ADC per stabilizzare i condensatori dopo il Deep Sleep */
        delay_ms(5);

        // --- 1. SVEGLIA MOTORI E FAI UN PASSO ---
        servo_enable();
        do_tracking(); // Fa 1 singolo passo come da Codice 1

        // --- 2. ASPETTA E LEGGI ENERGIA ---
        delay_ms(300);
        ina219_print();
        uart_puts("\r\n");

        if (current_phase == PHASE_DEMO) {
            demo_cycle_count++;
            if (demo_cycle_count >= DEMO_CYCLES) {
                current_phase = PHASE_INDUSTRIAL;
                rtc_set_wakeup_period(WAKEUP_IND_S);
            }
        }

        // --- 3. STAMPA SU SCHERMO ---
        oled_wake();

        /* Calcola i gradi con la formula del Codice 1 */
        int32_t pan_deg  = ((angle_pan - 500) * 180) / 2000;
        int32_t tilt_deg = ((angle_tilt - 500) * 180) / 2000;

        oled_print_telemetry(oled_vbat, oled_curr, oled_pwr, pan_deg, tilt_deg);
        delay_ms(4000);
        oled_sleep();

        // --- 4. SPEGNI E DORMI ---
        uart_puts("  >> Sleep...\r\n");
        servo_disable();

        TIM2_CR1  |= (1u << 0);
        TIM2_DIER |= (1u << 0);
        *(volatile uint32_t*)0xE000E100u |= (1u << 28);

        while (!(RTC_SR & (1u << 2))) {
            __asm volatile ("wfi" ::: "memory");
        }

        RTC_WPR = 0xCAu; RTC_WPR = 0x53u;
        RTC_CR  &= ~RTC_CR_WUTE;
        volatile uint32_t t = 100000u;
        while (!(RTC_ICSR & RTC_ICSR_WUTWF) && --t);
        RTC_SCR  = RTC_SCR_CWUTF;
        RTC_CR  |=  RTC_CR_WUTE;
        RTC_WPR  = 0xFFu;

        GPIOA_ODR |= (1u << 5);
        uart_puts("  << Wake!\r\n");
    }
}
