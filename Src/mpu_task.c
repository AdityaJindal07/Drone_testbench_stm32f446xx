/*
 * mpu_task.c
 *
 *  Created on: Oct 22, 2026
 *      Author: aditya-jindal007
 */


/*
 * mpu_task.c
 *
 * MPU6050 task: reads accel and gyro, Kalman-filters the accel, fuses roll and
 * pitch with a complementary filter, and streams "ax,ay,az,roll,pitch" over
 * USART2 for the Serial Plotter.
 *
 * The loop time (dt) used by the complementary filter is measured with the
 * DWT cycle counter on every call to MPU_Task_Run() and exported as g_dt.
 *
 * Pin Map:
 *   MPU6050 SCL -> PB8 (I2C1_SCL, AF4)   MPU6050 SDA -> PB9 (I2C1_SDA, AF4)
 *   ST-LINK VCP -> PA2 (USART2_TX, AF7)  115200 8N1
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_i2c_driver.h"
#include "stm32f446xx_uart_driver.h"
#include "Kalman.h"
#include "mpu_task.h"

/* =========================================================
 * Configuration
 * ========================================================= */

/* MPU6050 registers */
#define MPU6050_ADDR              (0x68U)
#define MPU6050_REG_SMPLRT_DIV    (0x19U)
#define MPU6050_REG_CONFIG        (0x1AU)
#define MPU6050_REG_GYRO_CONFIG   (0x1BU)
#define MPU6050_REG_ACCEL_CONFIG  (0x1CU)
#define MPU6050_REG_INT_ENABLE    (0x38U)
#define MPU6050_REG_ACCEL_XOUT_H  (0x3BU)
#define MPU6050_REG_PWR_MGMT_1    (0x6BU)
#define MPU6050_REG_WHO_AM_I      (0x75U)

#define MPU6050_WHO_AM_I_68       (0x68U)
#define MPU6050_WHO_AM_I_69       (0x69U)
#define MPU6050_WHO_AM_I_70       (0x70U)

#define MPU6050_BURST_LEN         (14U)       /* ACCEL(6) + TEMP(2) + GYRO(6) */

/* Sensor scaling for +/-2 g and +/-250 deg/s */
#define ACCEL_COUNTS_PER_G        (16384.0f)
#define GYRO_COUNTS_PER_DPS       (131.0f)

#define NUM_AXES                  (3U)
#define PLOT_CHANNELS             (5U)        /* ax_f, ay_f, az_f, roll, pitch */
#define LINE_SIZE                 (128U)

#define MPU_RAD_TO_DEG            (57.2957795f)

/*
 * Filter tuning (values from the original main.c):
 *   MPU_COMP_ALPHA : complementary filter weight of the gyro (0.9 .. 0.99)
 *   dt is measured every loop (see g_dt), no fixed constant is needed.
 */
#define MPU_KALMAN_Q              (0.01f)
#define MPU_KALMAN_R              (0.1f)
#define MPU_COMP_ALPHA            (0.96f)

/* Loop-time measurement (DWT cycle counter) */
#define MPU_CORE_CLK_HZ           (16000000.0f)
#define MPU_DT_MIN_S              (0.0005f)
#define MPU_DT_MAX_S              (0.1f)

#define DEMCR                     (*(volatile uint32_t *)0xE000EDFCUL)
#define DWT_CTRL                  (*(volatile uint32_t *)0xE0001000UL)
#define DWT_CYCCNT                (*(volatile uint32_t *)0xE0001004UL)

/* Coarse software delays (loop counts, not calibrated) */
#define MPU_WAKE_DELAY            (20000U)
#define MPU_REG_SETTLE_DELAY      (500U)
#define MPU_SAMPLE_DELAY          (0U)        /* was 80000U: no extra delay, keeps the loop fast */
#define MPU_CAL_SAMPLES           (200U)
#define MPU_CAL_SAMPLE_DELAY      (20000U)

/* =========================================================
 * Module state
 * ========================================================= */

typedef struct
{
    uint8_t reg;
    uint8_t value;
} MPU6050_RegWrite_t;

static const MPU6050_RegWrite_t k_mpu6050_config[] =
{
    { MPU6050_REG_SMPLRT_DIV,   0x07U },
    { MPU6050_REG_CONFIG,       0x06U },
    { MPU6050_REG_GYRO_CONFIG,  0x00U },
    { MPU6050_REG_ACCEL_CONFIG, 0x00U },
    { MPU6050_REG_INT_ENABLE,   0x00U },
};

static I2C_Handle_t   s_i2c1;
static USART_Handle_t s_usart2;
static Kalman1D_t     s_kf[NUM_AXES];

static float    s_plot[PLOT_CHANNELS];
static char     s_line[LINE_SIZE];
static float    s_gyro_bias[NUM_AXES];
static float    s_roll;
static float    s_pitch;
static uint8_t  s_ready;
static uint32_t s_last_cyc;

volatile float g_ax = 0.0f, g_ay = 0.0f, g_az = 0.0f;
volatile float g_gx = 0.0f, g_gy = 0.0f, g_gz = 0.0f;
volatile float g_roll = 0.0f, g_pitch = 0.0f;
volatile float g_dt = 0.01f;                  /* measured seconds per loop */

/* =========================================================
 * Utilities
 * ========================================================= */

static void Delay_Loop(volatile uint32_t count)
{
    while (count-- > 0U)
    {
        __asm volatile ("nop");
    }
}

/* Appends "<value, 3 decimals><term>" at buf[pos]; returns the new position. */
static size_t Append_Fixed3(char *buf, size_t size, size_t pos,
                            float val, const char *term)
{
    const int32_t  milli = (int32_t)(val * 1000.0f + ((val >= 0.0f) ? 0.5f : -0.5f));
    const uint32_t mag   = (milli < 0) ? (uint32_t)(-milli) : (uint32_t)milli;
    const char    *sign  = (milli < 0) ? "-" : "";
    const size_t   room  = size - pos;

    const int n = snprintf(buf + pos, room, "%s%lu.%03lu%s",
                           sign,
                           (unsigned long)(mag / 1000U),
                           (unsigned long)(mag % 1000U),
                           term);

    if ((n < 0) || ((size_t)n >= room))
    {
        return size - 1U;
    }

    return pos + (size_t)n;
}

/* Builds "v0,v1,...,vN\r\n" */
static void BuildPlotLine(char *line, size_t size, const float *vals, uint8_t count)
{
    size_t pos = 0U;

    line[0] = '\0';

    for (uint8_t i = 0U; i < count; i++)
    {
        const char *term = (i == (uint8_t)(count - 1U)) ? "\r\n" : ",";
        pos = Append_Fixed3(line, size, pos, vals[i], term);
    }
}

/* Static tilt from the accelerometer, in degrees. Guards avoid NaN on zero input. */
static void Calculate_Roll_Pitch(float ax, float ay, float az, float *roll, float *pitch)
{
    const float denominator = sqrtf((ay * ay) + (az * az));

    if ((fabsf(az) > 0.0001f) || (fabsf(ay) > 0.0001f))
    {
        *roll = atan2f(ay, az) * MPU_RAD_TO_DEG;
    }

    if (denominator > 0.0001f)
    {
        *pitch = atan2f(-ax, denominator) * MPU_RAD_TO_DEG;
    }
}

/* =========================================================
 * USART2 (via generic USART driver)
 * ========================================================= */

static void USART2_Setup(void)
{
    GPIO_Handle_t tx_pin;

    /* Early clock enable, see the note in the driver about clock settling */
    USART_PeriClockControl(USART2, ENABLE);

    /* PA2 -> USART2_TX (AF7) */
    GPIO_PeriClkControl(GPIOA, ENABLE);

    memset(&tx_pin, 0, sizeof(tx_pin));
    tx_pin.pGPIOx                             = GPIOA;
    tx_pin.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_2;
    tx_pin.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_ALTFN;
    tx_pin.GPIO_PinConfig.GPIO_PinAltFunMode  = 7U;
    tx_pin.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_PP;
    tx_pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    tx_pin.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_FAST;
    GPIO_Init(&tx_pin);

    memset(&s_usart2, 0, sizeof(s_usart2));
    s_usart2.pUSARTx                          = USART2;
    s_usart2.USART_Config.USART_Baud          = USART_STD_BAUD_115200;
    s_usart2.USART_Config.USART_Mode          = USART_MODE_ONLY_TX;
    s_usart2.USART_Config.USART_NoOfStopBits  = USART_STOPBITS_1;
    s_usart2.USART_Config.USART_ParityControl = USART_PARITY_DISABLE;
    s_usart2.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;
    USART_Init(&s_usart2);
}

static void USART2_WriteString(const char *s)
{
    (void)USART_SendData(&s_usart2,
                         (uint8_t *)(uintptr_t)s,
                         (uint32_t)strlen(s));
}

/* =========================================================
 * I2C1 (via generic I2C driver)
 * ========================================================= */

static void I2C1_Setup(void)
{
    GPIO_Handle_t i2c_pin;

    /* PB8 -> I2C1_SCL, PB9 -> I2C1_SDA (AF4, open-drain, pull-up) */
    GPIO_PeriClkControl(GPIOB, ENABLE);

    memset(&i2c_pin, 0, sizeof(i2c_pin));
    i2c_pin.pGPIOx                             = GPIOB;
    i2c_pin.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_ALTFN;
    i2c_pin.GPIO_PinConfig.GPIO_PinAltFunMode  = 4U;
    i2c_pin.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_OD;
    i2c_pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    i2c_pin.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_FAST;

    i2c_pin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_8;
    GPIO_Init(&i2c_pin);

    i2c_pin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_9;
    GPIO_Init(&i2c_pin);

    /* Clock first, then reset the peripheral to clear any stuck bus state */
    I2C_PeriClockControl(I2C1, ENABLE);
    I2C_DeInit(I2C1);

    memset(&s_i2c1, 0, sizeof(s_i2c1));
    s_i2c1.pI2Cx                      = I2C1;
    s_i2c1.I2C_Config.I2C_SCLSpeed    = I2C_SCL_SPEED_SM;   /* 100 kHz */
    s_i2c1.I2C_Config.I2C_ACKControl  = I2C_ACK_ENABLE;
    s_i2c1.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_2;
    I2C_Init(&s_i2c1);
}

/* =========================================================
 * MPU6050
 * ========================================================= */

static uint8_t MPU6050_WriteReg(uint8_t reg, uint8_t value)
{
    uint8_t frame[2] = { reg, value };

    return I2C_MasterSendData(&s_i2c1, frame, (uint32_t)sizeof(frame), MPU6050_ADDR);
}

static uint8_t MPU6050_ReadRegs(uint8_t start_reg, uint8_t *buffer, uint8_t length)
{
    const uint8_t status = I2C_MasterSendData(&s_i2c1, &start_reg, 1U, MPU6050_ADDR);

    if (status != I2C_STATUS_OK)
    {
        return status;
    }

    Delay_Loop(MPU_REG_SETTLE_DELAY);

    return I2C_MasterReceiveData(&s_i2c1, buffer, length, MPU6050_ADDR);
}

static uint8_t MPU6050_IsValidId(uint8_t who_am_i)
{
    return (uint8_t)((who_am_i == MPU6050_WHO_AM_I_68) ||
                     (who_am_i == MPU6050_WHO_AM_I_69) ||
                     (who_am_i == MPU6050_WHO_AM_I_70));
}

static int16_t MakeInt16(uint8_t high, uint8_t low)
{
    return (int16_t)(((uint16_t)high << 8U) | (uint16_t)low);
}

/* Wakes and configures the sensor; reports WHO_AM_I over the UART. */
static uint8_t MPU6050_Init(uint8_t *who_am_i)
{
    char    msg[64];
    uint8_t status;

    *who_am_i = 0U;

    (void)MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x00U);   /* wake up */
    Delay_Loop(MPU_WAKE_DELAY);

    for (size_t i = 0U; i < (sizeof(k_mpu6050_config) / sizeof(k_mpu6050_config[0])); i++)
    {
        (void)MPU6050_WriteReg(k_mpu6050_config[i].reg, k_mpu6050_config[i].value);
    }

    status = MPU6050_ReadRegs(MPU6050_REG_WHO_AM_I, who_am_i, 1U);

    (void)snprintf(msg, sizeof(msg), "I2C status=%u, WHO_AM_I=0x%02X\r\n",
                   (unsigned int)status, (unsigned int)*who_am_i);
    USART2_WriteString(msg);

    return status;
}

/* One sample: accel in g, gyro in deg/s (bias not removed). */
static uint8_t MPU6050_ReadScaled(float accel[NUM_AXES], float gyro[NUM_AXES])
{
    uint8_t raw[MPU6050_BURST_LEN];
    const uint8_t status = MPU6050_ReadRegs(MPU6050_REG_ACCEL_XOUT_H, raw, MPU6050_BURST_LEN);

    if (status != I2C_STATUS_OK)
    {
        return status;
    }

    accel[0] = (float)MakeInt16(raw[0],  raw[1])  / ACCEL_COUNTS_PER_G;
    accel[1] = (float)MakeInt16(raw[2],  raw[3])  / ACCEL_COUNTS_PER_G;
    accel[2] = (float)MakeInt16(raw[4],  raw[5])  / ACCEL_COUNTS_PER_G;

    gyro[0]  = (float)MakeInt16(raw[8],  raw[9])  / GYRO_COUNTS_PER_DPS;
    gyro[1]  = (float)MakeInt16(raw[10], raw[11]) / GYRO_COUNTS_PER_DPS;
    gyro[2]  = (float)MakeInt16(raw[12], raw[13]) / GYRO_COUNTS_PER_DPS;

    return I2C_STATUS_OK;
}

/* Averages the gyro with the board still to find its zero offset (deg/s). */
static void MPU6050_CalibrateGyro(void)
{
    float    accel[NUM_AXES];
    float    gyro[NUM_AXES];
    float    sum[NUM_AXES] = { 0.0f, 0.0f, 0.0f };
    uint32_t valid = 0U;

    USART2_WriteString("Calibrating gyro. Keep the board still...\r\n");

    for (uint32_t i = 0U; i < MPU_CAL_SAMPLES; i++)
    {
        if (MPU6050_ReadScaled(accel, gyro) == I2C_STATUS_OK)
        {
            for (uint8_t k = 0U; k < NUM_AXES; k++)
            {
                sum[k] += gyro[k];
            }
            valid++;
        }

        Delay_Loop(MPU_CAL_SAMPLE_DELAY);
    }

    for (uint8_t k = 0U; k < NUM_AXES; k++)
    {
        s_gyro_bias[k] = (valid > 0U) ? (sum[k] / (float)valid) : 0.0f;
    }
}

/* =========================================================
 * Public API
 * ========================================================= */

uint8_t MPU_Task_Init(void)
{
    uint8_t who_am_i = 0U;
    float   accel[NUM_AXES];
    float   gyro[NUM_AXES];

    s_ready = 0U;

    USART2_Setup();
    USART2_WriteString("\r\nSTM32F446RE - MPU6050 Real-Time Reader\r\n");
    USART2_WriteString("I2C1: PB8=SCL, PB9=SDA | USART2 TX: PA2 | 115200 8N1\r\n");

    for (uint8_t i = 0U; i < NUM_AXES; i++)
    {
        Kalman1D_Init(&s_kf[i], MPU_KALMAN_Q, MPU_KALMAN_R);
    }

    I2C1_Setup();
    (void)MPU6050_Init(&who_am_i);

    if (MPU6050_IsValidId(who_am_i) == 0U)
    {
        (void)snprintf(s_line, sizeof(s_line),
                       "MPU6050 not detected (WHO_AM_I=0x%02X). Check wiring/I2C driver.\r\n",
                       (unsigned int)who_am_i);
        USART2_WriteString(s_line);
        return MPU_TASK_ERR_NO_SENSOR;
    }

    MPU6050_CalibrateGyro();

    /* Start the fusion at the measured tilt */
    if (MPU6050_ReadScaled(accel, gyro) == I2C_STATUS_OK)
    {
        Calculate_Roll_Pitch(accel[0], accel[1], accel[2], &s_roll, &s_pitch);
        g_roll  = s_roll;
        g_pitch = s_pitch;
    }

    USART2_WriteString("MPU6050 detected. Streaming serial plotter data...\r\n");

    /* Start the cycle counter used to measure the loop time */
    DEMCR      |= (1UL << 24);
    DWT_CYCCNT  = 0U;
    DWT_CTRL   |= 1UL;
    s_last_cyc  = DWT_CYCCNT;

    s_ready = 1U;
    return MPU_TASK_OK;
}

void MPU_Task_Run(void)
{
    float accel[NUM_AXES];
    float gyro[NUM_AXES];
    float accel_roll;
    float accel_pitch;
    float dt;

    if (s_ready == 0U)
    {
        return;                         /* no sensor, nothing to do */
    }

    /* Measured time since the previous call, clamped to a sane range */
    {
        const uint32_t now = DWT_CYCCNT;

        dt         = (float)(now - s_last_cyc) / MPU_CORE_CLK_HZ;
        s_last_cyc = now;

        if (dt < MPU_DT_MIN_S) { dt = MPU_DT_MIN_S; }
        if (dt > MPU_DT_MAX_S) { dt = MPU_DT_MAX_S; }

        g_dt = dt;
    }

    if (MPU6050_ReadScaled(accel, gyro) == I2C_STATUS_OK)
    {
        /* 1. Kalman-filter the accelerometer */
        for (uint8_t i = 0U; i < NUM_AXES; i++)
        {
            s_plot[i] = Kalman1D_Update(&s_kf[i], accel[i]);
        }

        g_ax = s_plot[0];
        g_ay = s_plot[1];
        g_az = s_plot[2];

        /* 2. Remove the gyro bias */
        g_gx = gyro[0] - s_gyro_bias[0];
        g_gy = gyro[1] - s_gyro_bias[1];
        g_gz = gyro[2] - s_gyro_bias[2];

        /* 3. Static tilt from the filtered accelerometer */
        Calculate_Roll_Pitch(s_plot[0], s_plot[1], s_plot[2], &accel_roll, &accel_pitch);

        /* 4. Complementary filter: gyro for fast changes, accel for drift */
        s_roll  = (MPU_COMP_ALPHA * (s_roll  + g_gx * dt)) +
                  ((1.0f - MPU_COMP_ALPHA) * accel_roll);
        s_pitch = (MPU_COMP_ALPHA * (s_pitch + g_gy * dt)) +
                  ((1.0f - MPU_COMP_ALPHA) * accel_pitch);

        g_roll  = s_roll;
        g_pitch = s_pitch;

        /* 5. Output: ax_f, ay_f, az_f, roll, pitch */
        s_plot[3] = s_roll;
        s_plot[4] = s_pitch;

        BuildPlotLine(s_line, sizeof(s_line), s_plot, (uint8_t)PLOT_CHANNELS);
        USART2_WriteString(s_line);
    }
    else
    {
        USART2_WriteString("MPU6050 read failed\r\n");
    }

    Delay_Loop(MPU_SAMPLE_DELAY);
}
