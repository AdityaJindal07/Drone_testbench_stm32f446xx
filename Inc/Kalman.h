/*
 * Kalman.h
 *
 *  Created on: Oct 17, 2026
 *      Author: aditya-jindal007
 */

/*
 * kalman.h
 *
 * Minimal 1-D Kalman filter (header-only, no malloc, no libm).
 * Use one instance per axis (ax, ay, az).
 *
 * Model: the true value changes slowly (random walk)
 *   x[k] = x[k-1] + w,   w ~ N(0, Q)     <- process model
 *   z[k] = x[k]   + v,   v ~ N(0, R)     <- measurement (sensor reading)
 *
 * Tuning:
 *   R = measurement noise variance. Larger R -> trust the sensor less
 *       -> smoother output, more lag.
 *   Q = process noise variance. Larger Q -> allow faster changes
 *       -> follows the sensor more closely, less smoothing.
 *   Only the ratio Q/R really matters. Raise Q/R for faster response,
 *   lower it for more smoothing.
 */

#ifndef KALMAN_H_
#define KALMAN_H_

#include <stdint.h>

typedef struct
{
    float x;    /* State estimate (filtered value)          */
    float p;    /* Estimate error covariance                */
    float q;    /* Process noise variance                   */
    float r;    /* Measurement noise variance               */
    float k;    /* Last Kalman gain (useful for debugging)  */
    uint8_t initialized;
} Kalman1D_t;

/* Initialise the filter. Call once per axis before use. */
static inline void Kalman1D_Init(Kalman1D_t *kf, float q, float r)
{
    kf->x = 0.0f;
    kf->p = 1.0f;       /* Large initial uncertainty so it converges fast */
    kf->q = q;
    kf->r = r;
    kf->k = 0.0f;
    kf->initialized = 0U;
}

/* Feed one new measurement, get the filtered value back. */
static inline float Kalman1D_Update(Kalman1D_t *kf, float measurement)
{
    /* First sample: start from the measurement instead of 0 */
    if (kf->initialized == 0U)
    {
        kf->x = measurement;
        kf->initialized = 1U;
        return kf->x;
    }

    /* 1) Predict: state stays the same, uncertainty grows by Q */
    kf->p = kf->p + kf->q;

    /* 2) Kalman gain: how much to trust the new measurement (0..1) */
    kf->k = kf->p / (kf->p + kf->r);

    /* 3) Update: blend prediction and measurement */
    kf->x = kf->x + kf->k * (measurement - kf->x);

    /* 4) Update the uncertainty */
    kf->p = (1.0f - kf->k) * kf->p;

    return kf->x;
}

#endif /* KALMAN_H_ */
