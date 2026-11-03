/*
 * balance.c
 *
 *  Created on: Oct 26, 2026
 *      Author: aditya-jindal007
 */


/*
 * balance.c
 */

#include "balance.h"

static float Clamp(float v, float lo, float hi)
{
    if (v < lo) { return lo; }
    if (v > hi) { return hi; }
    return v;
}

void Balance_PID_Init(Balance_PID_t *p, float kp, float ki, float kd, float out_limit)
{
    p->Kp            = kp;
    p->Ki            = ki;
    p->Kd            = kd;
    p->Setpoint      = 0.0f;
    p->OutLimit      = out_limit;
    p->IntegralLimit = out_limit * 0.5f;
    p->Integral      = 0.0f;
}

void Balance_PID_Reset(Balance_PID_t *p)
{
    p->Integral = 0.0f;
}

float Balance_PID_Update(Balance_PID_t *p, float angle, float rate, float dt)
{
    const float error = p->Setpoint - angle;
    float       i_term;
    float       out;

    /* Integral with anti-windup (limit applies to the Ki*integral term) */
    p->Integral += error * dt;

    if (p->Ki > 0.0f)
    {
        const float i_max = p->IntegralLimit / p->Ki;
        p->Integral = Clamp(p->Integral, -i_max, i_max);
    }
    else
    {
        p->Integral = 0.0f;
    }

    i_term = p->Ki * p->Integral;

    /* Derivative on measurement: d(error)/dt = -rate */
    out = (p->Kp * error) + i_term - (p->Kd * rate);

    return Clamp(out, -p->OutLimit, p->OutLimit);
}
