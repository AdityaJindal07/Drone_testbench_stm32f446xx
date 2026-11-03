/*
 * main.c  -  Roll balancing with ESC/motors on a wooden log
 *
 * Motor A: TIM3_CH1 on PB4 (D5)      Motor B: TIM3_CH2 on PB5 (D4)
 * MPU6050 on I2C1 (PB8/PB9), debug stream on USART2 (PA2, 115200).
 *
 * Each loop: read roll and roll rate from the MPU, run a PID, and set the
 * motor throttle around a base throttle. With two motors they get opposite
 * corrections. With one motor (USE_MOTOR_B = 0) only motor A is driven.
 *
 * MODES (ESC_CALIBRATE_MODE)
 *   1 = one-time ESC throttle calibration (see ESC_Calibrate in esc.h):
 *       2000 us for 7 s (connect the LiPo now), then 1000 us for 5 s.
 *   0 = normal run (default): 1000 us while the ESC arms, then the MPU
 *       initializes, then the zero offset is measured, then the balance
 *       controller runs.
 *
 * HOW TO START A RUN (ESC_CALIBRATE_MODE = 0)
 *   1. Flash, terminate the debug session so the program runs on its own.
 *   2. Press reset, wait until the program is finished arming (about 10 s),
 *      keep the log still AND LEVEL during the MPU init and the zero
 *      offset measurement that follows it (about 1-2 s more).
 *   3. Connect the LiPo only when the signal is steady at 1000 us.
 *      (Same order that worked for you: LiPo connected while the program
 *      sits at the minimum pulse, then press reset.)
 *
 * ONE-MOTOR TEST (USE_MOTOR_B = 0)
 *   One motor cannot balance the log, but you can hold the log by hand:
 *     - motor A speeds up when the A end is LOW, slows when it is HIGH
 *     - smooth response, no twitching
 *   If the direction is wrong, flip A_END_LOW_WHEN_ROLL_POSITIVE.
 *
 * SAFETY
 *
 *   - If |roll| goes beyond TILT_CUTOFF_DEG the motors stop until reset.
 *   - The throttle of each motor can change by at most SLEW_STEP_PCT
 *     per loop, so one noisy sample cannot jerk a motor.
 *   - Keep the log restrained (padded stops) while tuning, and keep a
 *     way to disconnect the battery quickly.
 *
 * DIRECTION
 *   The MPU X axis must run along the log's pivot axis (roll = rotation
 *   about X). A_END_LOW_WHEN_ROLL_POSITIVE:
 *     1 = when g_roll is positive, the motor A end is the LOW end
 *     0 = when g_roll is positive, the motor B end is the LOW end
 */

#include <stdint.h>
#include <math.h>
#include "esc.h"
#include "mpu_task.h"
#include "balance.h"

/* ---- Settings you can change ---------------------------------- */
#define ESC_CALIBRATE_MODE             0   /* 0 = normal run, 1 = calibrate once */

#define USE_MOTOR_B                    1   /* 0 = only motor A connected, 1 = both */

#define A_END_LOW_WHEN_ROLL_POSITIVE   0

#define BASE_THROTTLE_PCT    17.0f   /* 1120 us */
#define MIN_THROTTLE_PCT      10.0f   /* 1080 us */
#define MAX_THROTTLE_PCT     25.0f   /* 1180 us */
#define MAX_CORRECTION_PCT    7.0f
#define TILT_CUTOFF_DEG      35.0f   /* tighter safety cutoff (was 40)       */

#define ROLL_KI   0.00f
#define ROLL_KP   0.05f
#define ROLL_KD   0.05f

/* Zero offset: average roll and rate at startup and subtract them.
 * The log must be still and level while this runs. */
#define ZERO_SETTLE_SAMPLES   500U   /* loops thrown away first              */
#define ZERO_AVG_SAMPLES      1000U   /* loops averaged                       */

/* Rate filter: rate_f = (1-alpha)*rate_f + alpha*rate.
 * 1.0 = no filtering (same as before). Try 0.2 if the motors buzz. */
#define RATE_FILTER_ALPHA     0.5f

/* Slew limit: max throttle change per loop, in percent.
 * Set to 100.0f to turn it off. Use a smaller value if the loop is fast. */
#define SLEW_STEP_PCT         2.5f

/* Per-motor trim added after the PID (fixes a slower motor).
 * 0.0 = no trim. Raise on the slow motor in 1% steps. */
#define TRIM_A_PCT            0.0f
#define TRIM_B_PCT            0.0f
/* --------------------------------------------------------------- */

static ESC_Handle_t motorA;
static const ESC_Config_t cfgA = { PWM_TIM3, PWM_CHANNEL_1, GPIOB, GPIO_PIN_NO_4 };

#if USE_MOTOR_B
static ESC_Handle_t motorB;
static const ESC_Config_t cfgB = { PWM_TIM3, PWM_CHANNEL_2, GPIOB, GPIO_PIN_NO_5 };
#endif

#if !ESC_CALIBRATE_MODE
static Balance_PID_t rollPID;

static float Clamp(float v, float lo, float hi)
{
    if (v < lo) { return lo; }
    if (v > hi) { return hi; }
    return v;
}

/* Move 'prev' toward 'target' by at most 'step' and return the new value */
static float Slew(float target, float prev, float step)
{
    if (target > prev + step) { return prev + step; }
    if (target < prev - step) { return prev - step; }
    return target;
}

static void Motors_Stop(void)
{
    (void)ESC_Stop(&motorA);
#if USE_MOTOR_B
    (void)ESC_Stop(&motorB);
#endif
}
#endif

int main(void)
{
#if ESC_CALIBRATE_MODE

    /* ======================= CALIBRATION MODE ======================= */
  #if USE_MOTOR_B
    ESC_Handle_t * const escs[] = { &motorA, &motorB };
  #else
    ESC_Handle_t * const escs[] = { &motorA };
  #endif

    if (ESC_Init(&motorA, &cfgA) != ESC_STATUS_OK) { while (1) {} }
  #if USE_MOTOR_B
    if (ESC_Init(&motorB, &cfgB) != ESC_STATUS_OK) { while (1) {} }
  #endif

    if (ESC_Calibrate(escs, (uint8_t)(sizeof(escs) / sizeof(escs[0]))) != ESC_STATUS_OK)
    {
        while (1) {}
    }

    while (1) {}                                   /* done: holds 1000 us */

#else

    /* ========================= NORMAL MODE ========================== */
    uint8_t stopped = 0U;
    float roll_zero = 0.0f;
    float rate_zero = 0.0f;
    float rate_f    = 0.0f;
    float a_prev    = BASE_THROTTLE_PCT;
  #if USE_MOTOR_B
    float b_prev    = BASE_THROTTLE_PCT;
  #endif

    /* 1. ESCs: start the signal at the minimum pulse (1000 us) */
    if (ESC_Init(&motorA, &cfgA) != ESC_STATUS_OK) { while (1) {} }
    if (ESC_Start(&motorA) != ESC_STATUS_OK)       { while (1) {} }
  #if USE_MOTOR_B
    if (ESC_Init(&motorB, &cfgB) != ESC_STATUS_OK) { while (1) {} }
    if (ESC_Start(&motorB) != ESC_STATUS_OK)       { while (1) {} }
  #endif

    /* 2. Hold the minimum pulse while the ESC arms */
    Motors_Stop();
    ESC_DelayMs(ESC_ARM_TIME_MS);

    /* 3. MPU init (takes a few seconds, keep the log still).
     *    The ESC keeps receiving the minimum pulse meanwhile. */
    if (MPU_Task_Init() != MPU_TASK_OK)
    {
        Motors_Stop();
        while (1) {}
    }

    /* 3b. Zero offset: keep the log still and level.
     *     The motors stay at the minimum pulse during this. */
    {
        uint16_t i;

        for (i = 0U; i < ZERO_SETTLE_SAMPLES; i++)
        {
            MPU_Task_Run();
        }

        for (i = 0U; i < ZERO_AVG_SAMPLES; i++)
        {
            MPU_Task_Run();
            rate_zero += g_gx;  /* Only calibrate gyro rate bias */
        }
        rate_zero /= (float)ZERO_AVG_SAMPLES;
        roll_zero = 0.0f;       /* Always use Earth gravity vector for 0 deg */
    }

    /* 4. Controller */
    Balance_PID_Init(&rollPID, ROLL_KP, ROLL_KI, ROLL_KD, MAX_CORRECTION_PCT);

    /* 5. Control loop (MPU_Task_Run reads the sensor once per call) */
    while (1)
    {
        float roll;
        float rate;
        float dt;
        float out;
        float a;
  #if USE_MOTOR_B
        float b;
  #endif

        MPU_Task_Run();

        if (stopped != 0U)
        {
            continue;
        }

        roll = g_roll - roll_zero;     /* angle with startup zero removed */
        rate = g_gx   - rate_zero;     /* roll rate, deg/s, zero removed  */
        dt   = g_dt;                   /* measured loop time, seconds     */

        /* Light low-pass on the rate (alpha 1.0 = off) */
        rate_f = (1.0f - RATE_FILTER_ALPHA) * rate_f + RATE_FILTER_ALPHA * rate;
        rate   = rate_f;

        /* Fell too far: stop and stay stopped until reset */
        if (fabsf(roll) > TILT_CUTOFF_DEG)
        {
            Motors_Stop();
            stopped = 1U;
            continue;
        }

        out = Balance_PID_Update(&rollPID, roll, rate, dt);

  #if A_END_LOW_WHEN_ROLL_POSITIVE
        a = BASE_THROTTLE_PCT - out;
    #if USE_MOTOR_B
        b = BASE_THROTTLE_PCT + out;
    #endif
  #else
        a = BASE_THROTTLE_PCT + out;
    #if USE_MOTOR_B
        b = BASE_THROTTLE_PCT - out;
    #endif
  #endif

        a = Clamp(a + TRIM_A_PCT, MIN_THROTTLE_PCT, MAX_THROTTLE_PCT);
        a = Slew(a, a_prev, SLEW_STEP_PCT);
        a_prev = a;
        (void)ESC_SetThrottlePercent(&motorA, a);

  #if USE_MOTOR_B
        b = Clamp(b + TRIM_B_PCT, MIN_THROTTLE_PCT, MAX_THROTTLE_PCT);
        b = Slew(b, b_prev, SLEW_STEP_PCT);
        b_prev = b;
        (void)ESC_SetThrottlePercent(&motorB, b);
  #endif
    }

#endif /* ESC_CALIBRATE_MODE */
}
