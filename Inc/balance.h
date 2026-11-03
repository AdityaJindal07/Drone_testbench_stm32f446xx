/*
 * balance.h
 *
 *  Created on: Oct 26, 2026
 *      Author: aditya-jindal007
 */

/*
 * balance.h
 *
 * Small PID controller for one angle (roll). Uses the gyro rate directly as
 * the derivative term, so it does not depend on loop-time noise.
 *
 *   error  = Setpoint - angle
 *   output = Kp*error + Ki*integral(error) - Kd*rate
 *
 * Output is a throttle correction in percent, limited to +/- OutLimit.
 */

#ifndef BALANCE_H_
#define BALANCE_H_

typedef struct
{
    float Kp;               /* % throttle per degree of error          */
    float Ki;               /* % throttle per degree-second            */
    float Kd;               /* % throttle per deg/s of rotation rate   */
    float Setpoint;         /* target angle, degrees (0 = level)       */
    float OutLimit;         /* output clamp, +/- % throttle            */
    float IntegralLimit;    /* clamp for the Ki*integral term, % throttle */
    float Integral;         /* running integral (degree-seconds)       */
} Balance_PID_t;

void  Balance_PID_Init(Balance_PID_t *p, float kp, float ki, float kd, float out_limit);
void  Balance_PID_Reset(Balance_PID_t *p);

/* angle: degrees, rate: deg/s (same axis), dt: seconds. Returns % throttle correction. */
float Balance_PID_Update(Balance_PID_t *p, float angle, float rate, float dt);

#endif /* BALANCE_H_ */
