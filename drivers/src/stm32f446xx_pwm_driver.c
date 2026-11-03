/*

 * stm32f446xx_pwm_driver.c

 *

 *  Created on: Oct 22, 2026

 *      Author: aditya-jindal007

 */

/*

 * stm32f446xx_pwm_driver.c

 *

 * PWM output driver for STM32F446xx timers TIM1..TIM5.

 * See stm32f446xx_pwm_driver.h for usage and notes.

 */

#include <string.h>

#include "stm32f446xx_pwm_driver.h"

/* ---------------------------------------------------------

 * Timer register bits

 * --------------------------------------------------------- */

#define TIM_CR1_CEN             (1U << 0)

#define TIM_CR1_ARPE            (1U << 7)

#define TIM_EGR_UG              (1U << 0)

#define TIM_BDTR_MOE            (1U << 15)

/* CCMRx, per-channel byte: [CCxS 1:0][OCxFE 2][OCxPE 3][OCxM 6:4][OCxCE 7] */

#define TIM_CCMR_OCPE           (1U << 3)

#define TIM_CCMR_OCM_SHIFT      4U

#define TIM_CCMR_OCM_FORCE_INACTIVE   4U    /* 100 */

#define TIM_CCMR_OCM_PWM1             6U    /* 110 */

#define TIM_CCMR_CHANNEL_MASK   0xFFU

/* CCER, 4 bits per channel: CCxE at bit 4*(ch-1), CCxP at bit 4*(ch-1)+1 */

#define TIM_CCER_CCE(ch)        (1U << (4U * ((ch) - 1U)))

#define TIM_CCER_CCP(ch)        (1U << ((4U * ((ch) - 1U)) + 1U))

/* ---------------------------------------------------------

 * Alternate functions

 * --------------------------------------------------------- */

#define PWM_AF_TIM1_TIM2        1U

#define PWM_AF_TIM3_TIM5        2U

/* ---------------------------------------------------------

 * Private helpers

 * --------------------------------------------------------- */

static uint8_t PWM_IsValidTimer(const PWM_TIM_RegDef_t *t)

{

    return (uint8_t)((t == PWM_TIM1) || (t == PWM_TIM2) || (t == PWM_TIM3) ||

                     (t == PWM_TIM4) || (t == PWM_TIM5));

}

static uint8_t PWM_Is32Bit(const PWM_TIM_RegDef_t *t)

{

    return (uint8_t)((t == PWM_TIM2) || (t == PWM_TIM5));

}

static uint32_t PWM_GetTimerClock(const PWM_TIM_RegDef_t *t)

{

    return (t == PWM_TIM1) ? PWM_APB2_TIMER_CLK_HZ : PWM_APB1_TIMER_CLK_HZ;

}

static uint8_t PWM_DefaultAltFunc(const PWM_TIM_RegDef_t *t)

{

    return (uint8_t)(((t == PWM_TIM1) || (t == PWM_TIM2)) ? PWM_AF_TIM1_TIM2

                                                          : PWM_AF_TIM3_TIM5);

}

/*

 * Enables the timer clock. The read-back makes sure the enable has reached the

 * peripheral before its registers are touched (STM32F4 errata).

 */

static void PWM_TimerClockEnable(const PWM_TIM_RegDef_t *t)

{

    volatile uint32_t readback;

    if (t == PWM_TIM1)

    {

        RCC->APB2ENR |= (1U << 0);

        readback = RCC->APB2ENR;

    }

    else

    {

        if (t == PWM_TIM2)      { RCC->APB1ENR |= (1U << 0); }

        else if (t == PWM_TIM3) { RCC->APB1ENR |= (1U << 1); }

        else if (t == PWM_TIM4) { RCC->APB1ENR |= (1U << 2); }

        else                    { RCC->APB1ENR |= (1U << 3); }   /* TIM5 */

        readback = RCC->APB1ENR;

    }

    (void)readback;

}

/* Sets the OCxM field of one channel and keeps its compare preload enabled. */

static void PWM_SetOutputMode(PWM_TIM_RegDef_t *t, uint8_t ch, uint32_t mode)

{

    volatile uint32_t *ccmr = (ch <= 2U) ? &t->CCMR1 : &t->CCMR2;

    const uint32_t shift    = (uint32_t)(((ch - 1U) & 1U) * 8U);

    uint32_t value          = *ccmr;

    value &= ~((uint32_t)TIM_CCMR_CHANNEL_MASK << shift);

    value |= ((mode << TIM_CCMR_OCM_SHIFT) | TIM_CCMR_OCPE) << shift;

    *ccmr = value;

}

/*

 * Chooses PSC and ARR for the requested frequency. Uses no prescaler whenever

 * the period fits the counter, which gives the finest duty resolution.

 * Writes PSC and ARR; the caller generates the update event.

 */

static uint8_t PWM_ConfigTimebase(PWM_Handle_t *h, uint32_t frequency)

{

    PWM_TIM_RegDef_t *t = h->pTIMx;

    const uint32_t clk      = PWM_GetTimerClock(t);

    const uint64_t counts   = PWM_Is32Bit(t) ? 0x100000000ULL : 0x10000ULL;

    uint64_t ticks;

    uint64_t psc;

    uint64_t period;

    if (frequency == 0U)

    {

        return PWM_STATUS_ERROR;

    }

    /* Timer ticks per output period, rounded to nearest */

    ticks = ((uint64_t)clk + ((uint64_t)frequency / 2U)) / (uint64_t)frequency;

    if (ticks < 2U)

    {

        return PWM_STATUS_ERROR;            /* frequency too high for this clock */

    }

    psc = (ticks - 1U) / counts;

    if (psc > 0xFFFFULL)

    {

        return PWM_STATUS_ERROR;            /* frequency too low */

    }

    period = ticks / (psc + 1U);            /* ARR + 1 */

    t->PSC = (uint32_t)psc;

    t->ARR = (uint32_t)(period - 1U);

    h->TimerClkHz = clk;

    h->Prescaler  = (uint32_t)psc;

    h->Period     = (uint32_t)period;

    return PWM_STATUS_OK;

}

/* Writes the compare value for this channel from the stored duty. */

static void PWM_WriteDuty(PWM_Handle_t *h)

{

    const uint64_t ccr = (((uint64_t)h->Period * h->DutyHundredths) + 5000U) / 10000U;

    h->pTIMx->CCR[h->PWM_Config.PWM_Channel - 1U] = (uint32_t)ccr;

}

/*

 * Compare values are preloaded, so a stopped counter never copies them to the

 * active registers. Force an update event in that case.

 */

static void PWM_LoadIfStopped(PWM_TIM_RegDef_t *t)

{

    if ((t->CR1 & TIM_CR1_CEN) == 0U)

    {

        t->EGR = TIM_EGR_UG;

    }

}

static uint32_t PWM_PercentToHundredths(float percent)

{

    if (!(percent >= 0.0f))

    {

        percent = 0.0f;                     /* also catches NaN */

    }

    if (percent > 100.0f)

    {

        percent = 100.0f;

    }

    return (uint32_t)((percent * 100.0f) + 0.5f);

}

static uint8_t PWM_HandleIsUsable(const PWM_Handle_t *h)

{

    return (uint8_t)((h != 0) && (h->Period != 0U));

}

/* ---------------------------------------------------------

 * Public API

 * --------------------------------------------------------- */

uint8_t PWM_Init(PWM_Handle_t *pHandle)

{

    PWM_TIM_RegDef_t *t;

    GPIO_Handle_t     pin;

    uint8_t           ch;

    uint8_t           alt;

    if (pHandle == 0)

    {

        return PWM_STATUS_ERROR;

    }

    t   = pHandle->pTIMx;

    ch  = pHandle->PWM_Config.PWM_Channel;

    alt = pHandle->PWM_Config.PWM_AltFunc;

    pHandle->Period = 0U;                   /* not usable until init succeeds */

    if ((PWM_IsValidTimer(t) == 0U) ||
        (ch < PWM_CHANNEL_1) || (ch > PWM_CHANNEL_4) ||
        (pHandle->PWM_Config.PWM_Polarity > PWM_POLARITY_LOW) ||
        (pHandle->PWM_Config.PWM_pGPIOx == 0) ||
        (pHandle->PWM_Config.PWM_PinNumber > GPIO_PIN_NO_15))
    {
        return PWM_STATUS_ERROR;
    }

    /* Resolve AUTO before checking the valid alternate-function range. */
    if (alt == PWM_AF_AUTO)
    {
        alt = PWM_DefaultAltFunc(t);
    }

    if (alt > 15U)
    {
        return PWM_STATUS_ERROR;
    }

    /* 1. Timer clock */

    PWM_TimerClockEnable(t);

    /* 2. Output pin through the GPIO driver */

    GPIO_PeriClkControl(pHandle->PWM_Config.PWM_pGPIOx, ENABLE);

    memset(&pin, 0, sizeof(pin));

    pin.pGPIOx                             = pHandle->PWM_Config.PWM_pGPIOx;

    pin.GPIO_PinConfig.GPIO_PinNumber      = pHandle->PWM_Config.PWM_PinNumber;

    pin.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_ALTFN;

    pin.GPIO_PinConfig.GPIO_PinAltFunMode  = alt;

    pin.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_PP;

    pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    pin.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_HIGH;

    GPIO_Init(&pin);

    /* 3. Timebase: preload ARR, then PSC and ARR from the frequency */

    t->CR1 |= TIM_CR1_ARPE;

    if (PWM_ConfigTimebase(pHandle, pHandle->PWM_Config.PWM_FrequencyHz) != PWM_STATUS_OK)

    {

        return PWM_STATUS_ERROR;

    }

    /* 4. Channel: PWM output idle (forced inactive) until PWM_Start() */

    PWM_SetOutputMode(t, ch, TIM_CCMR_OCM_FORCE_INACTIVE);

    if (pHandle->PWM_Config.PWM_Polarity == PWM_POLARITY_LOW)

    {

        t->CCER |= TIM_CCER_CCP(ch);

    }

    else

    {

        t->CCER &= ~TIM_CCER_CCP(ch);

    }

    t->CCER |= TIM_CCER_CCE(ch);

    /* 5. Initial duty, then load all preloaded registers */

    pHandle->DutyHundredths = PWM_PercentToHundredths(pHandle->PWM_Config.PWM_DutyPercent);

    PWM_WriteDuty(pHandle);

    /* 6. TIM1 is an advanced timer: its outputs need the main output enable */

    if (t == PWM_TIM1)

    {

        t->BDTR |= TIM_BDTR_MOE;

    }

    t->EGR = TIM_EGR_UG;

    return PWM_STATUS_OK;

}

uint8_t PWM_Start(PWM_Handle_t *pHandle)

{

    if (PWM_HandleIsUsable(pHandle) == 0U)

    {

        return PWM_STATUS_ERROR;

    }

    PWM_SetOutputMode(pHandle->pTIMx, pHandle->PWM_Config.PWM_Channel, TIM_CCMR_OCM_PWM1);

    pHandle->pTIMx->CR1 |= TIM_CR1_CEN;

    return PWM_STATUS_OK;

}

void PWM_Stop(PWM_Handle_t *pHandle)

{

    if (PWM_HandleIsUsable(pHandle) == 0U)

    {

        return;

    }

    PWM_SetOutputMode(pHandle->pTIMx, pHandle->PWM_Config.PWM_Channel,

                      TIM_CCMR_OCM_FORCE_INACTIVE);

}

uint8_t PWM_SetDutyPercent(PWM_Handle_t *pHandle, float DutyPercent)

{

    if (PWM_HandleIsUsable(pHandle) == 0U)

    {

        return PWM_STATUS_ERROR;

    }

    pHandle->DutyHundredths = PWM_PercentToHundredths(DutyPercent);

    PWM_WriteDuty(pHandle);

    PWM_LoadIfStopped(pHandle->pTIMx);

    return PWM_STATUS_OK;

}

uint8_t PWM_SetPulseWidthUs(PWM_Handle_t *pHandle, uint32_t PulseWidthUs)

{

    uint64_t ccr;

    if (PWM_HandleIsUsable(pHandle) == 0U)

    {

        return PWM_STATUS_ERROR;

    }

    /* ticks = us * (clock / (PSC + 1)) / 1e6 */

    ccr = ((uint64_t)PulseWidthUs * pHandle->TimerClkHz) /

          (((uint64_t)pHandle->Prescaler + 1U) * 1000000ULL);

    if (ccr > pHandle->Period)

    {

        ccr = pHandle->Period;

    }

    pHandle->DutyHundredths = (uint32_t)((ccr * 10000U) / pHandle->Period);

    pHandle->pTIMx->CCR[pHandle->PWM_Config.PWM_Channel - 1U] = (uint32_t)ccr;

    PWM_LoadIfStopped(pHandle->pTIMx);

    return PWM_STATUS_OK;

}

uint8_t PWM_SetFrequency(PWM_Handle_t *pHandle, uint32_t FrequencyHz)

{

    if (PWM_HandleIsUsable(pHandle) == 0U)

    {

        return PWM_STATUS_ERROR;

    }

    if (PWM_ConfigTimebase(pHandle, FrequencyHz) != PWM_STATUS_OK)

    {

        return PWM_STATUS_ERROR;

    }

    pHandle->PWM_Config.PWM_FrequencyHz = FrequencyHz;

    /* Keep this channel's duty percent on the new period */

    PWM_WriteDuty(pHandle);

    /*

     * The update event reloads PSC, ARR and all compare registers at once and

     * restarts the counter, so a running output sees one short or long cycle.

     */

    pHandle->pTIMx->EGR = TIM_EGR_UG;

    return PWM_STATUS_OK;

}

uint32_t PWM_GetFrequency(const PWM_Handle_t *pHandle)

{

    if (PWM_HandleIsUsable(pHandle) == 0U)

    {

        return 0U;

    }

    return (uint32_t)((uint64_t)pHandle->TimerClkHz /

                      (((uint64_t)pHandle->Prescaler + 1U) * pHandle->Period));

}
