#include "pwm_task.h"
#include "stm32f446xx_pwm_driver.h"

/* PB4 (TIM3 CH1 / Arduino D12) avoids SPI1 / MPU6050 conflict on PA7 */
#define PWM_TASK_TIMER          PWM_TIM3
#define PWM_TASK_CHANNEL        PWM_CHANNEL_1
#define PWM_TASK_PORT           GPIOB
#define PWM_TASK_PIN            GPIO_PIN_NO_4
#define PWM_TASK_FREQUENCY_HZ   (50U)
#define PWM_TASK_DUTY_PERCENT   (5.0f) /* 5.0% = 1000us initial arming signal */

static PWM_Handle_t s_pwm;

uint8_t PWM_Task_Init(void)
{
    s_pwm.pTIMx                      = PWM_TASK_TIMER;
    s_pwm.PWM_Config.PWM_Channel     = PWM_TASK_CHANNEL;
    s_pwm.PWM_Config.PWM_pGPIOx      = PWM_TASK_PORT;
    s_pwm.PWM_Config.PWM_PinNumber   = PWM_TASK_PIN;
    s_pwm.PWM_Config.PWM_AltFunc     = PWM_AF_AUTO;
    s_pwm.PWM_Config.PWM_FrequencyHz = PWM_TASK_FREQUENCY_HZ;
    s_pwm.PWM_Config.PWM_DutyPercent = PWM_TASK_DUTY_PERCENT;
    s_pwm.PWM_Config.PWM_Polarity    = PWM_POLARITY_HIGH;

    if (PWM_Init(&s_pwm) != PWM_STATUS_OK)
    {
        return PWM_STATUS_ERROR;
    }

    return PWM_Start(&s_pwm);
}

uint8_t PWM_Task_SetDutyPercent(float DutyPercent)
{
    return PWM_SetDutyPercent(&s_pwm, DutyPercent);
}

uint8_t PWM_Task_SetPulseWidthUs(uint32_t PulseWidthUs)
{
    return PWM_SetPulseWidthUs(&s_pwm, PulseWidthUs);
}
