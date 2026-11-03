/*
 * esc.c
 *
 *  Created on: Oct 25, 2026
 *      Author: aditya-jindal007
 */


/*
 * esc.c
 *
 * ESC API on top of stm32f446xx_pwm_driver. See esc.h.
 */

#include "esc.h"

/* ---------------------------------------------------------
 * Private helpers
 * --------------------------------------------------------- */

/* The FPU must be on before any float code runs (PWM_Init uses float).
 * Without it the CPU faults inside PWM_Init(). Safe to call more than once. */
#define CPACR_ADDR            0xE000ED88UL
#define CPACR_CP10_CP11_FULL  ((3UL << 20U) | (3UL << 22U))

static void ESC_FPU_Enable(void)
{
    *(volatile uint32_t *)CPACR_ADDR |= CPACR_CP10_CP11_FULL;
    __asm volatile ("dsb");
    __asm volatile ("isb");
}

static uint8_t ESC_IsUsable(const ESC_Handle_t *pESC)
{
    return (uint8_t)(pESC != 0);
}

/* ---------------------------------------------------------
 * Public API
 * --------------------------------------------------------- */

void ESC_DelayMs(uint32_t ms)
{
    SysTick->LOAD = (ESC_CORE_CLK_HZ / 1000U) - 1U;   /* 1 ms period */
    SysTick->VAL  = 0U;
    SysTick->CTRL = (1U << 0) | (1U << 2);            /* enable, processor clock */

    for (uint32_t i = 0U; i < ms; i++)
    {
        while ((SysTick->CTRL & (1U << 16)) == 0U)
        {
            /* wait for COUNTFLAG */
        }
    }

    SysTick->CTRL = 0U;
}

uint8_t ESC_Init(ESC_Handle_t *pESC, const ESC_Config_t *pCfg)
{
    if ((pESC == 0) || (pCfg == 0))
    {
        return ESC_STATUS_ERROR;
    }

    ESC_FPU_Enable();

    pESC->pwm.pTIMx                      = pCfg->pTIMx;
    pESC->pwm.PWM_Config.PWM_FrequencyHz = 50U;
    pESC->pwm.PWM_Config.PWM_DutyPercent = 5.0f;          /* 1000 us at 50 Hz */
    pESC->pwm.PWM_Config.PWM_Channel     = pCfg->Channel;
    pESC->pwm.PWM_Config.PWM_Polarity    = PWM_POLARITY_HIGH;
    pESC->pwm.PWM_Config.PWM_pGPIOx      = pCfg->pGPIOx;
    pESC->pwm.PWM_Config.PWM_PinNumber   = pCfg->PinNumber;
    pESC->pwm.PWM_Config.PWM_AltFunc     = PWM_AF_AUTO;

    pESC->pulse_us = ESC_PULSE_MIN_US;

    return PWM_Init(&pESC->pwm);
}

uint8_t ESC_Start(ESC_Handle_t *pESC)
{
    if (ESC_IsUsable(pESC) == 0U)
    {
        return ESC_STATUS_ERROR;
    }

    return PWM_Start(&pESC->pwm);
}

uint8_t ESC_SetPulseUs(ESC_Handle_t *pESC, uint32_t PulseUs)
{
    if (ESC_IsUsable(pESC) == 0U)
    {
        return ESC_STATUS_ERROR;
    }

    if (PulseUs < ESC_PULSE_MIN_US) { PulseUs = ESC_PULSE_MIN_US; }
    if (PulseUs > ESC_PULSE_MAX_US) { PulseUs = ESC_PULSE_MAX_US; }

    if (PWM_SetPulseWidthUs(&pESC->pwm, PulseUs) != PWM_STATUS_OK)
    {
        return ESC_STATUS_ERROR;
    }

    pESC->pulse_us = PulseUs;
    return ESC_STATUS_OK;
}

uint8_t ESC_SetThrottlePercent(ESC_Handle_t *pESC, float ThrottlePercent)
{
    uint32_t us;

    if (!(ThrottlePercent >= 0.0f)) { ThrottlePercent = 0.0f; }     /* also NaN */
    if (ThrottlePercent > 100.0f)   { ThrottlePercent = 100.0f; }

    us = ESC_PULSE_MIN_US +
         (uint32_t)(((float)(ESC_PULSE_MAX_US - ESC_PULSE_MIN_US) *
                     (ThrottlePercent / 100.0f)) + 0.5f);

    return ESC_SetPulseUs(pESC, us);
}

uint8_t ESC_Stop(ESC_Handle_t *pESC)
{
    return ESC_SetPulseUs(pESC, ESC_PULSE_MIN_US);
}

uint32_t ESC_GetPulseUs(const ESC_Handle_t *pESC)
{
    return (pESC != 0) ? pESC->pulse_us : 0U;
}

uint8_t ESC_Calibrate(ESC_Handle_t * const escs[], uint8_t count)
{
    uint8_t i;

    if ((escs == 0) || (count == 0U))
    {
        return ESC_STATUS_ERROR;
    }

    /* Full throttle first, then start the signal */
    for (i = 0U; i < count; i++)
    {
        if (ESC_SetPulseUs(escs[i], ESC_PULSE_MAX_US) != ESC_STATUS_OK) { return ESC_STATUS_ERROR; }
        if (ESC_Start(escs[i]) != ESC_STATUS_OK)                        { return ESC_STATUS_ERROR; }
    }

    ESC_DelayMs(ESC_CAL_MAX_HOLD_MS);          /* connect the LiPo(s) now */

    /* Zero throttle: ESCs save the endpoints */
    for (i = 0U; i < count; i++)
    {
        if (ESC_Stop(escs[i]) != ESC_STATUS_OK) { return ESC_STATUS_ERROR; }
    }

    ESC_DelayMs(ESC_CAL_MIN_HOLD_MS);

    return ESC_STATUS_OK;
}
