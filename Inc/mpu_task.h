/*
 * mpu_task.h
 *
 *  Created on: Oct 22, 2026
 *      Author: aditya-jindal007
 */

#ifndef INC_MPU_TASK_H_
#define INC_MPU_TASK_H_

#include <stdint.h>

#define MPU_TASK_OK             (0U)
#define MPU_TASK_ERR_NO_SENSOR  (1U)

/* Live values for the debugger (Live Expressions) */
extern volatile float g_ax, g_ay, g_az;       /* Kalman-filtered accel, g       */
extern volatile float g_gx, g_gy, g_gz;       /* gyro, deg/s, bias removed      */
extern volatile float g_roll, g_pitch;        /* fused tilt, degrees            */
extern volatile float g_dt;                   /* measured seconds per loop      */

/*
 * Sets up USART2 and I2C1, wakes the MPU6050, calibrates the gyro and starts
 * the filters. Returns MPU_TASK_OK, or MPU_TASK_ERR_NO_SENSOR if no MPU6050
 * answers. It does not block in that case, and MPU_Task_Run() then does nothing.
 */
uint8_t MPU_Task_Init(void);

/* One sample: read, filter, fuse, print one line over USART2. Call in a loop. */
void MPU_Task_Run(void);

#endif /* INC_MPU_TASK_H_ */
