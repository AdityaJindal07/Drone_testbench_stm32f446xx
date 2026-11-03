/*
 * esc.h
 *
 *  Created on: Oct 25, 2026
 *      Author: aditya-jindal007
 */

/*
 * esc.h
 *
 * Simple API for hobby brushless ESCs (50 Hz, 1000..2000 us pulse).
 * Built on top of stm32f446xx_pwm_driver. One ESC_Handle_t per motor.
 *
 * Typical use:
 *     ESC_Handle_t motor;
 *     ESC_Config_t cfg = { PWM_TIM3, PWM_CHANNEL_1, GPIOB, GPIO_PIN_NO_4 };
 *
 *     ESC_Init(&motor, &cfg);      // output idle, pulse = 1000 us
 *     ESC_Start(&motor);           // signal on the pin (1000 us)
 *     ESC_DelayMs(ESC_ARM_TIME_MS);// ESC arms
 *     ESC_SetThrottlePercent(&motor, 20.0f);
 *
 * Two ESCs can share one timer (they share the 50 Hz frequency), e.g.
 * TIM3 CH1 = PB4 and TIM3 CH2 = PB5.
 */

#ifndef ESC_H_
#define ESC_H_

#include <stdint.h>
#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_pwm_driver.h"

/* ---- Constants ------------------------------------------------ */
#define ESC_PULSE_MIN_US     1000U    /* zero throttle              */
#define ESC_PULSE_MAX_US     2000U    /* full throttle              */
#define ESC_ARM_TIME_MS      5000U    /* time at 1000 us to arm     */

#define ESC_CAL_MAX_HOLD_MS  7000U    /* connect the LiPo in this time */
#define ESC_CAL_MIN_HOLD_MS  5000U    /* ESC saves the endpoints       */

/* Core clock for ESC_DelayMs (default HSI 16 MHz). Override if you change it. */
#ifndef ESC_CORE_CLK_HZ
#define ESC_CORE_CLK_HZ      16000000U
#endif

#define ESC_STATUS_OK        PWM_STATUS_OK
#define ESC_STATUS_ERROR     PWM_STATUS_ERROR

/* ---- Types ---------------------------------------------------- */
typedef struct
{
    PWM_TIM_RegDef_t *pTIMx;      /* PWM_TIM1 .. PWM_TIM5                */
    uint8_t           Channel;    /* PWM_CHANNEL_1 .. PWM_CHANNEL_4      */
    GPIO_RegDef_t    *pGPIOx;     /* port of the signal pin, e.g. GPIOB  */
    uint8_t           PinNumber;  /* GPIO_PIN_NO_x                       */
} ESC_Config_t;

typedef struct
{
    PWM_Handle_t pwm;             /* underlying PWM channel              */
    uint32_t     pulse_us;        /* last pulse width that was set       */
} ESC_Handle_t;

/* ---- API ------------------------------------------------------ */

/* Enables the FPU, configures the pin and 50 Hz timer. Output stays idle
 * until ESC_Start(). Pulse is preset to 1000 us. */
uint8_t ESC_Init(ESC_Handle_t *pESC, const ESC_Config_t *pCfg);

/* Starts the signal. Sends whatever pulse is currently set (1000 us after init). */
uint8_t ESC_Start(ESC_Handle_t *pESC);

/* Sets the pulse width in us, clamped to 1000..2000. */
uint8_t ESC_SetPulseUs(ESC_Handle_t *pESC, uint32_t PulseUs);

/* Sets throttle 0.0 .. 100.0 % (maps to 1000..2000 us). */
uint8_t ESC_SetThrottlePercent(ESC_Handle_t *pESC, float ThrottlePercent);

/* Zero throttle (1000 us). */
uint8_t ESC_Stop(ESC_Handle_t *pESC);

/* Last pulse width that was set, in us. */
uint32_t ESC_GetPulseUs(const ESC_Handle_t *pESC);

/*
 * One-time throttle calibration for one or more ESCs at once.
 *   1. Disconnect the ESC battery(ies). STM32 stays on USB power.
 *   2. Call ESC_Init() on each ESC, then ESC_Calibrate().
 *   3. Connect the battery(ies) within ESC_CAL_MAX_HOLD_MS.
 *   4. The ESCs save the endpoints. Calibration is then done.
 * Blocking. The ESCs end at 1000 us.
 */
uint8_t ESC_Calibrate(ESC_Handle_t * const escs[], uint8_t count);

/* Blocking millisecond delay (SysTick polling). */
void ESC_DelayMs(uint32_t ms);

#endif /* ESC_H_ */
