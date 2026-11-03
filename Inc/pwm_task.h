#ifndef PWM_TASK_H_
#define PWM_TASK_H_

#include <stdint.h>

/**
 * @brief  Initializes PWM (TIM3 CH1 on PB4 / Arduino D12, 50 Hz).
 * @return 0 on success, non-zero on error.
 */
uint8_t PWM_Task_Init(void);

/**
 * @brief  Updates PWM output using floating-point duty percentage (0.0f - 100.0f).
 */
uint8_t PWM_Task_SetDutyPercent(float DutyPercent);

/**
 * @brief  Updates PWM output using explicit pulse width in microseconds.
 * @param  PulseWidthUs: Pulse duration in microseconds (1000us = zero throttle, 2000us = full throttle).
 */
uint8_t PWM_Task_SetPulseWidthUs(uint32_t PulseWidthUs);

#endif /* PWM_TASK_H_ */
