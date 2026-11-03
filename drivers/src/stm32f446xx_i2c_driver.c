/*
 * stm32f446xx_i2c_driver.c
 *
 * Register-level I2C driver for STM32F446xx
 */

#include "stm32f446xx_i2c_driver.h"

/* Polling timeout: adjust for your system clock and application. */
#define I2C_TIMEOUT_VALUE    1000000U

/* Private function declarations */
static void I2C_ClearADDRFlag(I2C_RegDef_t *pI2Cx);
static void I2C_ClearErrorFlags(I2C_RegDef_t *pI2Cx);
static uint8_t I2C_WaitForFlag(I2C_RegDef_t *pI2Cx, uint32_t FlagName);

/* =========================================================
 * Peripheral clock control
 * ========================================================= */
void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
    if (EnorDi == ENABLE)
    {
        if (pI2Cx == I2C1)
        {
            I2C1_PCLK_EN();
        }
        else if (pI2Cx == I2C2)
        {
            I2C2_PCLK_EN();
        }
        else if (pI2Cx == I2C3)
        {
            I2C3_PCLK_EN();
        }
    }
    else
    {
        if (pI2Cx == I2C1)
        {
            I2C1_PCLK_DI();
        }
        else if (pI2Cx == I2C2)
        {
            I2C2_PCLK_DI();
        }
        else if (pI2Cx == I2C3)
        {
            I2C3_PCLK_DI();
        }
    }
}

/* =========================================================
 * Peripheral enable / disable
 * ========================================================= */
void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
    if (EnorDi == ENABLE)
    {
        pI2Cx->CR1 |= I2C_CR1_PE;
    }
    else
    {
        pI2Cx->CR1 &= ~I2C_CR1_PE;
    }
}

/* =========================================================
 * ACK control
 * ========================================================= */
void I2C_ManageAcking(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
    if (EnorDi == I2C_ACK_ENABLE)
    {
        pI2Cx->CR1 |= I2C_CR1_ACK;
    }
    else
    {
        pI2Cx->CR1 &= ~I2C_CR1_ACK;
    }
}

/* =========================================================
 * Initialize I2C peripheral
 * ========================================================= */
void I2C_Init(I2C_Handle_t *pI2CHandle)
{
    I2C_RegDef_t *pI2Cx;
    uint32_t pclk1;
    uint32_t ccr;
    uint32_t trise;
    uint32_t freqMHz;
    uint32_t speed;

    if (pI2CHandle == 0)
    {
        return;
    }

    pI2Cx = pI2CHandle->pI2Cx;

    if ((pI2Cx != I2C1) &&
        (pI2Cx != I2C2) &&
        (pI2Cx != I2C3))
    {
        return;
    }

    speed = pI2CHandle->I2C_Config.I2C_SCLSpeed;
    pclk1 = I2C_PCLK1_FREQ;

    if ((pclk1 < 2000000U) ||
        (pclk1 > 42000000U) ||
        ((pclk1 % 1000000U) != 0U))
    {
        return;
    }

    if ((speed != I2C_SCL_SPEED_SM) &&
        (speed != I2C_SCL_SPEED_FM))
    {
        return;
    }

    if ((pI2CHandle->I2C_Config.I2C_FMDutyCycle != I2C_FM_DUTY_2) &&
        (pI2CHandle->I2C_Config.I2C_FMDutyCycle != I2C_FM_DUTY_16_9))
    {
        return;
    }

    /* Enable peripheral clock */
    I2C_PeriClockControl(pI2Cx, ENABLE);

    /* Disable peripheral before setting timings */
    I2C_PeripheralControl(pI2Cx, DISABLE);

    freqMHz = pclk1 / 1000000U;

    /* CR2 FREQ field: APB1 clock in MHz */
    pI2Cx->CR2 &= ~(0x3FU);
    pI2Cx->CR2 |= (freqMHz & 0x3FU);

    pI2Cx->CCR = 0U;

    if (speed == I2C_SCL_SPEED_SM)
    {
        ccr = pclk1 / (2U * speed);
        if (ccr < 4U)
        {
            ccr = 4U;
        }
        pI2Cx->CCR = ccr & 0x0FFFU;
        trise = freqMHz + 1U;
    }
    else
    {
        if (pI2CHandle->I2C_Config.I2C_FMDutyCycle == I2C_FM_DUTY_2)
        {
            ccr = pclk1 / (3U * speed);
            if (ccr == 0U)
            {
                ccr = 1U;
            }
            pI2Cx->CCR = (1U << 15) | (ccr & 0x0FFFU);
        }
        else
        {
            ccr = pclk1 / (25U * speed);
            if (ccr == 0U)
            {
                ccr = 1U;
            }
            pI2Cx->CCR = (1U << 15) | (1U << 14) | (ccr & 0x0FFFU);
        }

        trise = ((freqMHz * 300U) / 1000U) + 1U;
    }

    pI2Cx->TRISE = trise & 0x3FU;

    /* Configure ACK */
    I2C_ManageAcking(pI2Cx, pI2CHandle->I2C_Config.I2C_ACKControl);

    /* Enable I2C */
    I2C_PeripheralControl(pI2Cx, ENABLE);
}

/* =========================================================
 * Deinitialize I2C peripheral
 * ========================================================= */
void I2C_DeInit(I2C_RegDef_t *pI2Cx)
{
    if (pI2Cx == I2C1)
    {
        RCC->APB1RSTR |= (1U << 21);
        RCC->APB1RSTR &= ~(1U << 21);
    }
    else if (pI2Cx == I2C2)
    {
        RCC->APB1RSTR |= (1U << 22);
        RCC->APB1RSTR &= ~(1U << 22);
    }
    else if (pI2Cx == I2C3)
    {
        RCC->APB1RSTR |= (1U << 23);
        RCC->APB1RSTR &= ~(1U << 23);
    }
}

/* =========================================================
 * Get I2C flag status
 * ========================================================= */
uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName)
{
    if ((pI2Cx->SR1 & FlagName) != 0U)
    {
        return SET;
    }
    return RESET;
}

/* =========================================================
 * Wait for a flag with timeout and error detection (FIXED)
 * ========================================================= */
static uint8_t I2C_WaitForFlag(I2C_RegDef_t *pI2Cx, uint32_t FlagName)
{
    uint32_t timeout = I2C_TIMEOUT_VALUE;
    uint32_t sr1_val;

    while (timeout > 0U)
    {
        /* Read SR1 ONCE per loop iteration to avoid flag clearing glitches */
        sr1_val = pI2Cx->SR1;

        if ((sr1_val & I2C_FLAG_BERR) != 0U)
        {
            return I2C_STATUS_ERROR;
        }
        if ((sr1_val & I2C_FLAG_ARLO) != 0U)
        {
            return I2C_STATUS_ERROR;
        }
        if ((sr1_val & I2C_FLAG_OVR) != 0U)
        {
            return I2C_STATUS_ERROR;
        }
        if ((sr1_val & I2C_FLAG_AF) != 0U)
        {
            return I2C_STATUS_NACK;
        }
        if ((sr1_val & FlagName) != 0U)
        {
            return I2C_STATUS_OK;
        }

        timeout--;
    }
    return I2C_STATUS_TIMEOUT;
}

/* =========================================================
 * Clear error flags (FIXED)
 * ========================================================= */
static void I2C_ClearErrorFlags(I2C_RegDef_t *pI2Cx)
{
    /* Clear flags by writing 0 to active error status bits */
    pI2Cx->SR1 &= ~(I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_AF | I2C_FLAG_OVR | I2C_FLAG_TIMEOUT);
}

/* =========================================================
 * Clear ADDR flag
 * ========================================================= */
static void I2C_ClearADDRFlag(I2C_RegDef_t *pI2Cx)
{
    volatile uint32_t dummyRead;
    dummyRead = pI2Cx->SR1;
    dummyRead = pI2Cx->SR2;
    (void)dummyRead;
}

/* =========================================================
 * Generate START condition
 * ========================================================= */
uint8_t I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx)
{
    pI2Cx->CR1 |= I2C_CR1_START;
    return I2C_WaitForFlag(pI2Cx, I2C_FLAG_SB);
}

/* =========================================================
 * Generate STOP condition
 * ========================================================= */
void I2C_GenerateStopCondition(I2C_RegDef_t *pI2Cx)
{
    pI2Cx->CR1 |= I2C_CR1_STOP;
}

/* =========================================================
 * Address phase: master transmitter
 * ========================================================= */
uint8_t I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr)
{
    uint8_t addr_byte = (SlaveAddr << 1U) & 0xFEU;
    pI2Cx->DR = addr_byte;
    return I2C_WaitForFlag(pI2Cx, I2C_FLAG_ADDR);
}

/* =========================================================
 * Address phase: master receiver
 * ========================================================= */
uint8_t I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr)
{
    uint8_t addr_byte = ((SlaveAddr << 1U) | 1U);
    pI2Cx->DR = addr_byte;
    return I2C_WaitForFlag(pI2Cx, I2C_FLAG_ADDR);
}

/* =========================================================
 * Master transmit
 * ========================================================= */
uint8_t I2C_MasterSendData(
    I2C_Handle_t *pI2CHandle,
    uint8_t *pTxBuffer,
    uint32_t Len,
    uint8_t SlaveAddr
)
{
    I2C_RegDef_t *pI2Cx;
    uint32_t i;
    uint8_t status;

    if ((pI2CHandle == 0) || (pTxBuffer == 0) || (Len == 0U))
    {
        return I2C_STATUS_ERROR;
    }

    pI2Cx = pI2CHandle->pI2Cx;

    I2C_ClearErrorFlags(pI2Cx);

    /* 1. Generate START */
    status = I2C_GenerateStartCondition(pI2Cx);
    if (status != I2C_STATUS_OK)
    {
        goto transmit_error;
    }

    /* 2. Execute Address Write Phase */
    status = I2C_ExecuteAddressPhaseWrite(pI2Cx, SlaveAddr);
    if (status != I2C_STATUS_OK)
    {
        goto transmit_error;
    }

    /* 3. Clear ADDR flag */
    I2C_ClearADDRFlag(pI2Cx);

    /* 4. Send Data Bytes */
    for (i = 0U; i < Len; i++)
    {
        status = I2C_WaitForFlag(pI2Cx, I2C_FLAG_TXE);
        if (status != I2C_STATUS_OK)
        {
            goto transmit_error;
        }
        pI2Cx->DR = pTxBuffer[i];
    }

    /* 5. Wait for BTF */
    status = I2C_WaitForFlag(pI2Cx, I2C_FLAG_BTF);
    if (status != I2C_STATUS_OK)
    {
        goto transmit_error;
    }

    /* 6. Generate STOP */
    I2C_GenerateStopCondition(pI2Cx);

    return I2C_STATUS_OK;

transmit_error:
    I2C_GenerateStopCondition(pI2Cx);
    I2C_ClearErrorFlags(pI2Cx);
    return status;
}

/* =========================================================
 * Master receive
 * ========================================================= */
uint8_t I2C_MasterReceiveData(
    I2C_Handle_t *pI2CHandle,
    uint8_t *pRxBuffer,
    uint32_t Len,
    uint8_t SlaveAddr
)
{
    I2C_RegDef_t *pI2Cx;
    uint32_t remaining;
    uint8_t status;
    const uint32_t POS_BIT = (1U << 11U); /* CR1.POS */

    if ((pI2CHandle == 0) ||
        (pRxBuffer == 0) ||
        (Len == 0U) ||
        (SlaveAddr > 0x7FU))
    {
        return I2C_STATUS_ERROR;
    }

    pI2Cx = pI2CHandle->pI2Cx;

    if ((pI2Cx != I2C1) &&
        (pI2Cx != I2C2) &&
        (pI2Cx != I2C3))
    {
        return I2C_STATUS_ERROR;
    }

    remaining = Len;

    I2C_ClearErrorFlags(pI2Cx);
    pI2Cx->CR1 &= ~POS_BIT;
    I2C_ManageAcking(pI2Cx, I2C_ACK_ENABLE);

    if (Len == 2U)
    {
        pI2Cx->CR1 |= POS_BIT;
    }

    status = I2C_GenerateStartCondition(pI2Cx);
    if (status != I2C_STATUS_OK)
    {
        goto receive_error;
    }

    status = I2C_ExecuteAddressPhaseRead(pI2Cx, SlaveAddr);
    if (status != I2C_STATUS_OK)
    {
        goto receive_error;
    }

    /* 1-byte read */
    if (Len == 1U)
    {
        I2C_ManageAcking(pI2Cx, I2C_ACK_DISABLE);
        I2C_ClearADDRFlag(pI2Cx);
        I2C_GenerateStopCondition(pI2Cx);

        status = I2C_WaitForFlag(pI2Cx, I2C_FLAG_RXNE);
        if (status != I2C_STATUS_OK)
        {
            goto receive_error;
        }

        *pRxBuffer = (uint8_t)pI2Cx->DR;
        remaining = 0U;
    }
    /* 2-byte read */
    else if (Len == 2U)
    {
        I2C_ManageAcking(pI2Cx, I2C_ACK_DISABLE);
        I2C_ClearADDRFlag(pI2Cx);

        status = I2C_WaitForFlag(pI2Cx, I2C_FLAG_BTF);
        if (status != I2C_STATUS_OK)
        {
            goto receive_error;
        }

        I2C_GenerateStopCondition(pI2Cx);

        *pRxBuffer = (uint8_t)pI2Cx->DR;
        pRxBuffer++;
        *pRxBuffer = (uint8_t)pI2Cx->DR;
        pRxBuffer++;
        remaining = 0U;
    }
    /* 3+ byte read */
    else
    {
        pI2Cx->CR1 &= ~POS_BIT;
        I2C_ManageAcking(pI2Cx, I2C_ACK_ENABLE);
        I2C_ClearADDRFlag(pI2Cx);

        while (remaining > 3U)
        {
            status = I2C_WaitForFlag(pI2Cx, I2C_FLAG_RXNE);
            if (status != I2C_STATUS_OK)
            {
                goto receive_error;
            }

            *pRxBuffer = (uint8_t)pI2Cx->DR;
            pRxBuffer++;
            remaining--;
        }

        status = I2C_WaitForFlag(pI2Cx, I2C_FLAG_BTF);
        if (status != I2C_STATUS_OK)
        {
            goto receive_error;
        }

        I2C_ManageAcking(pI2Cx, I2C_ACK_DISABLE);

        *pRxBuffer = (uint8_t)pI2Cx->DR;
        pRxBuffer++;
        remaining--;

        status = I2C_WaitForFlag(pI2Cx, I2C_FLAG_BTF);
        if (status != I2C_STATUS_OK)
        {
            goto receive_error;
        }

        I2C_GenerateStopCondition(pI2Cx);

        *pRxBuffer = (uint8_t)pI2Cx->DR;
        pRxBuffer++;
        remaining--;

        *pRxBuffer = (uint8_t)pI2Cx->DR;
        pRxBuffer++;
        remaining--;
    }

    pI2Cx->CR1 &= ~POS_BIT;
    I2C_ManageAcking(pI2Cx, pI2CHandle->I2C_Config.I2C_ACKControl);

    return I2C_STATUS_OK;

receive_error:
    I2C_GenerateStopCondition(pI2Cx);
    pI2Cx->CR1 &= ~POS_BIT;
    I2C_ManageAcking(pI2Cx, pI2CHandle->I2C_Config.I2C_ACKControl);
    I2C_ClearErrorFlags(pI2Cx);

    return status;
}
