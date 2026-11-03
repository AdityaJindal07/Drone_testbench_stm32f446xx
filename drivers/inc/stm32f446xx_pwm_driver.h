/*
 * stm32f446xx_pwm_driver.h
 *
 *  Created on: Oct 22, 2026
 *      Author: aditya-jindal007
 */

/*
 * stm32f446xx_pwm_driver.h
 *
 * PWM output driver for STM32F446xx timers TIM1..TIM5 (PWM mode 1, edge-aligned).
 * Pin setup goes through the project GPIO driver.
 *
 * How it works
 *   - Every timer has ONE frequency, shared by all of its channels.
 *   - Each channel has its own duty cycle (compare value).
 *   - Frequency is set with a prescaler (PSC) and auto-reload (ARR) chosen to
 *     give the finest duty resolution the timer can offer for that frequency.
 *   - Duty and frequency changes are preloaded: they take effect at the end of
 *     the current period, so the output does not glitch while running.
 *
 * Timer clock
 *   The driver cannot detect your clock tree. Set the timer input clocks with
 *   PWM_APB1_TIMER_CLK_HZ (TIM2..TIM5) and PWM_APB2_TIMER_CLK_HZ (TIM1), for
 *   example in the compiler's defined-symbols list. Defaults are for the reset
 *   state (HSI 16 MHz, no bus prescalers).
 *   If an APB prescaler is not 1, the timer clock is 2 x PCLK, not PCLK.
 *
 * Pins and alternate functions (NUCLEO-F446RE)
 *   Timer  CH1   CH2   CH3   CH4   AF
 *   TIM1   PA8   PA9   PA10  PA11  AF1
 *   TIM2   PA0   PA1   PA2   PA3   AF1
 *   TIM3   PA6   PA7   PB0   PB1   AF2
 *   TIM4   PB6   PB7   PB8   PB9   AF2
 *   TIM5   PA0   PA1   PA2   PA3   AF2
 *   Use PWM_AF_AUTO to let the driver pick the AF from the timer. The driver
 *   does not check that the pin really belongs to that timer channel.
 *   TIM2 CH3 (PA2) is the USART2 TX pin and TIM4 CH3/CH4 (PB8/PB9) are the I2C1
 *   pins in the MPU6050 project; avoid those when using this driver there.
 *
 * Resolution
 *   TIM2 and TIM5 are 32-bit; TIM1, TIM3 and TIM4 are 16-bit.
 *   At 16 MHz and 50 Hz on a 16-bit timer the tick is 0.3125 us.
 */

#ifndef INC_STM32F446XX_PWM_DRIVER_H_
#define INC_STM32F446XX_PWM_DRIVER_H_

#include <stdint.h>
#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"

/* =========================================================
 * Timer register map (TIM1..TIM5 common layout)
 * ========================================================= */

typedef struct
{
    volatile uint32_t CR1;       /* 0x00 */
    volatile uint32_t CR2;       /* 0x04 */
    volatile uint32_t SMCR;      /* 0x08 */
    volatile uint32_t DIER;      /* 0x0C */
    volatile uint32_t SR;        /* 0x10 */
    volatile uint32_t EGR;       /* 0x14 */
    volatile uint32_t CCMR1;     /* 0x18 */
    volatile uint32_t CCMR2;     /* 0x1C */
    volatile uint32_t CCER;      /* 0x20 */
    volatile uint32_t CNT;       /* 0x24 */
    volatile uint32_t PSC;       /* 0x28 */
    volatile uint32_t ARR;       /* 0x2C */
    volatile uint32_t RCR;       /* 0x30 (TIM1 only) */
    volatile uint32_t CCR[4];    /* 0x34..0x40 compare registers CH1..CH4 */
    volatile uint32_t BDTR;      /* 0x44 (TIM1 only) */
    volatile uint32_t DCR;       /* 0x48 */
    volatile uint32_t DMAR;      /* 0x4C */
    volatile uint32_t OR;        /* 0x50 */
} PWM_TIM_RegDef_t;

#define PWM_TIM1    ((PWM_TIM_RegDef_t *)TIM1_BASEADDR)
#define PWM_TIM2    ((PWM_TIM_RegDef_t *)TIM2_BASEADDR)
#define PWM_TIM3    ((PWM_TIM_RegDef_t *)TIM3_BASEADDR)
#define PWM_TIM4    ((PWM_TIM_RegDef_t *)TIM4_BASEADDR)
#define PWM_TIM5    ((PWM_TIM_RegDef_t *)TIM5_BASEADDR)

/* =========================================================
 * Timer input clocks in Hz (override at build time if needed)
 * ========================================================= */

#ifndef PWM_APB1_TIMER_CLK_HZ
#define PWM_APB1_TIMER_CLK_HZ   (16000000UL)   /* 16 MHz default HSI */  /* TIM2, TIM3, TIM4, TIM5 */
#endif

#ifndef PWM_APB2_TIMER_CLK_HZ
#define PWM_APB2_TIMER_CLK_HZ    16000000U   /* TIM1 */
#endif

/* =========================================================
 * Options
 * ========================================================= */

#define PWM_CHANNEL_1           1U
#define PWM_CHANNEL_2           2U
#define PWM_CHANNEL_3           3U
#define PWM_CHANNEL_4           4U

#define PWM_POLARITY_HIGH       0U      /* pin is high during the duty time    */
#define PWM_POLARITY_LOW        1U      /* pin is low during the duty time     */

#define PWM_AF_AUTO             0U      /* pick the alternate function from the timer */

/* Return values */
#define PWM_STATUS_OK           0U
#define PWM_STATUS_ERROR        1U

/* =========================================================
 * Configuration and handle
 * ========================================================= */

typedef struct
{
    uint32_t       PWM_FrequencyHz;     /* output frequency of the timer          */
    float          PWM_DutyPercent;     /* initial duty, 0.0 .. 100.0             */
    uint8_t        PWM_Channel;         /* PWM_CHANNEL_1 .. PWM_CHANNEL_4         */
    uint8_t        PWM_Polarity;        /* PWM_POLARITY_HIGH / PWM_POLARITY_LOW   */
    GPIO_RegDef_t *PWM_pGPIOx;          /* port of the output pin, e.g. GPIOA     */
    uint8_t        PWM_PinNumber;       /* GPIO_PIN_NO_x                          */
    uint8_t        PWM_AltFunc;         /* alternate function number or PWM_AF_AUTO */
} PWM_Config_t;

typedef struct
{
    PWM_TIM_RegDef_t *pTIMx;            /* PWM_TIM1 .. PWM_TIM5                   */
    PWM_Config_t      PWM_Config;

    /* Filled by the driver, do not modify */
    uint32_t          TimerClkHz;       /* timer input clock                      */
    uint32_t          Prescaler;        /* value written to PSC (divide by +1)    */
    uint32_t          Period;           /* ARR + 1, counter ticks per period      */
    uint32_t          DutyHundredths;   /* current duty in 0.01 % units (0..10000) */
} PWM_Handle_t;

/* =========================================================
 * Driver APIs
 * ========================================================= */

/*
 * Enables the timer clock, configures the pin through the GPIO driver, sets
 * the timer frequency and the channel. The output is left idle (inactive
 * level). Call PWM_Start() to begin generating the signal.
 *
 * Calling PWM_Init() for a second channel of a timer that already runs at a
 * different frequency changes the frequency of the whole timer. Re-set the
 * duty of the first channel afterwards.
 */
uint8_t PWM_Init(PWM_Handle_t *pHandle);

/* Starts the channel and the timer counter. */
uint8_t PWM_Start(PWM_Handle_t *pHandle);

/*
 * Forces the channel to its inactive level. The counter keeps running so other
 * channels of the same timer are not affected. PWM_Start() resumes the signal.
 */
void PWM_Stop(PWM_Handle_t *pHandle);

/* Sets the duty cycle, 0.0 .. 100.0 percent (values outside are clamped). */
uint8_t PWM_SetDutyPercent(PWM_Handle_t *pHandle, float DutyPercent);

/* Sets the high time in microseconds (clamped to the period). Useful for servos and ESCs. */
uint8_t PWM_SetPulseWidthUs(PWM_Handle_t *pHandle, uint32_t PulseWidthUs);

/*
 * Changes the timer frequency. This channel keeps its duty percent. Other
 * channels of the same timer keep their old compare values and must be set
 * again.
 */
uint8_t PWM_SetFrequency(PWM_Handle_t *pHandle, uint32_t FrequencyHz);

/* Frequency actually produced (integer rounding of PSC/ARR), in Hz. 0 if not initialised. */
uint32_t PWM_GetFrequency(const PWM_Handle_t *pHandle);

#endif /* INC_STM32F446XX_PWM_DRIVER_H_ */
