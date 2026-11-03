/*
 * stm32f446xx_spi_driver.c
 *
 *  Created on: Oct 15, 2026
 *      Author: aditya-jindal007
 */


/*
 * stm32f446xx_spi_driver.c
 *
 * Register-level SPI driver for STM32F446xx
 */

#include "stm32f446xx_spi_driver.h"


#define SPI_TIMEOUT_VALUE    1000000U

/* CR1 bit positions */

#define SPI_CR1_CPHA         (1U << 0)
#define SPI_CR1_CPOL         (1U << 1)
#define SPI_CR1_MSTR         (1U << 2)
#define SPI_CR1_BR           (7U << 3)
#define SPI_CR1_SPE          (1U << 6)
#define SPI_CR1_LSBFIRST     (1U << 7)
#define SPI_CR1_SSI          (1U << 8)
#define SPI_CR1_SSM          (1U << 9)
#define SPI_CR1_RXONLY       (1U << 10)
#define SPI_CR1_DFF          (1U << 11)
#define SPI_CR1_BIDIOE       (1U << 14)
#define SPI_CR1_BIDIMODE     (1U << 15)


/* =========================================================
 * Private function declarations
 * ========================================================= */

static uint8_t SPI_IsValidPeripheral(SPI_RegDef_t *pSPIx);

static uint8_t SPI_WaitForFlag(
    SPI_RegDef_t *pSPIx,
    uint32_t FlagName,
    uint8_t Status
);

static uint8_t SPI_WriteFrame(
    SPI_Handle_t *pSPIHandle,
    uint16_t data
);

static uint16_t SPI_ReadFrame(
    SPI_Handle_t *pSPIHandle
);

static uint8_t SPI_WaitUntilNotBusy(
    SPI_RegDef_t *pSPIx
);


/* =========================================================
 * Validate SPI peripheral
 * ========================================================= */

static uint8_t SPI_IsValidPeripheral(SPI_RegDef_t *pSPIx)
{
    if ((pSPIx == SPI1) ||
        (pSPIx == SPI2) ||
        (pSPIx == SPI3) ||
        (pSPIx == SPI4))
    {
        return 1U;
    }

    return 0U;
}


/* =========================================================
 * SPI peripheral clock control
 * ========================================================= */

void SPI_PeriClockControl(
    SPI_RegDef_t *pSPIx,
    uint8_t EnorDi
)
{
    if (SPI_IsValidPeripheral(pSPIx) == 0U)
    {
        return;
    }

    if (EnorDi == ENABLE)
    {
        if (pSPIx == SPI1)
        {
            SPI1_PCLK_EN();
        }
        else if (pSPIx == SPI2)
        {
            SPI2_PCLK_EN();
        }
        else if (pSPIx == SPI3)
        {
            SPI3_PCLK_EN();
        }
        else if (pSPIx == SPI4)
        {
            SPI4_PCLK_EN();
        }

    }
}


/* =========================================================
 * SPI peripheral enable / disable
 * ========================================================= */

void SPI_PeripheralControl(
    SPI_RegDef_t *pSPIx,
    uint8_t EnorDi
)
{
    if (pSPIx == 0)
    {
        return;
    }

    if (EnorDi == ENABLE)
    {
        pSPIx->CR1 |= SPI_CR1_SPE;
    }
    else
    {
        pSPIx->CR1 &= ~SPI_CR1_SPE;
    }
}


/* =========================================================
 * Configure software slave management
 * ========================================================= */

void SPI_SSIConfig(
    SPI_RegDef_t *pSPIx,
    uint8_t EnorDi
)
{
    if (pSPIx == 0)
    {
        return;
    }

    if (EnorDi == ENABLE)
    {
        pSPIx->CR1 |= SPI_CR1_SSI;
    }
    else
    {
        pSPIx->CR1 &= ~SPI_CR1_SSI;
    }
}


/* =========================================================
 * Configure hardware slave output enable
 * ========================================================= */

void SPI_SSOEConfig(
    SPI_RegDef_t *pSPIx,
    uint8_t EnorDi
)
{
    if (pSPIx == 0)
    {
        return;
    }

    if (EnorDi == ENABLE)
    {
        pSPIx->CR2 |= (1U << 2);
    }
    else
    {
        pSPIx->CR2 &= ~(1U << 2);
    }
}


/* =========================================================
 * Get SPI flag status
 * ========================================================= */

uint8_t SPI_GetFlagStatus(
    SPI_RegDef_t *pSPIx,
    uint32_t FlagName
)
{
    if (pSPIx == 0)
    {
        return RESET;
    }

    if ((pSPIx->SR & FlagName) != 0U)
    {
        return SET;
    }

    return RESET;
}


/* =========================================================
 * Wait for a flag
 *
 * Status = SET: wait until flag becomes set.
 * Status = RESET: wait until flag becomes clear.
 * ========================================================= */

static uint8_t SPI_WaitForFlag(
    SPI_RegDef_t *pSPIx,
    uint32_t FlagName,
    uint8_t Status
)
{
    uint32_t timeout = SPI_TIMEOUT_VALUE;

    while (timeout > 0U)
    {
        if (Status == SET)
        {
            if ((pSPIx->SR & FlagName) != 0U)
            {
                return SPI_STATUS_OK;
            }
        }
        else
        {
            if ((pSPIx->SR & FlagName) == 0U)
            {
                return SPI_STATUS_OK;
            }
        }

        timeout--;
    }

    return SPI_STATUS_TIMEOUT;
}


/* =========================================================
 * Wait until SPI is not busy
 * ========================================================= */

static uint8_t SPI_WaitUntilNotBusy(SPI_RegDef_t *pSPIx)
{
    return SPI_WaitForFlag(
        pSPIx,
        SPI_FLAG_BSY,
        RESET
    );
}


/* =========================================================
 * Initialize SPI
 * ========================================================= */

void SPI_Init(SPI_Handle_t *pSPIHandle)
{
    SPI_RegDef_t *pSPIx;
    uint32_t cr1 = 0U;

    if (pSPIHandle == 0)
    {
        return;
    }

    pSPIx = pSPIHandle->pSPIx;

    if (SPI_IsValidPeripheral(pSPIx) == 0U)
    {
        return;
    }

    /* Validate configuration. */

    if ((pSPIHandle->SPIConfig.SPI_DeviceMode >
         SPI_DEVICE_MODE_MASTER) ||
        (pSPIHandle->SPIConfig.SPI_BusConfig <
         SPI_BUS_CONFIG_FD) ||
        (pSPIHandle->SPIConfig.SPI_BusConfig >
         SPI_BUS_CONFIG_SIMPLEX_RX) ||
        (pSPIHandle->SPIConfig.SPI_SclkSpeed >
         SPI_SCLK_SPEED_DIV256) ||
        (pSPIHandle->SPIConfig.SPI_CPOL >
         SPI_CPOL_HIGH) ||
        (pSPIHandle->SPIConfig.SPI_CPHA >
         SPI_CPHA_HIGH) ||
        (pSPIHandle->SPIConfig.SPI_DFF >
         SPI_DFF_16BITS) ||
        (pSPIHandle->SPIConfig.SPI_SSM >
         SPI_SSM_EN))
    {
        return;
    }

    /* Enable peripheral clock. */

    SPI_PeriClockControl(pSPIx, ENABLE);

    /* Disable SPI before changing its configuration. */

    SPI_PeripheralControl(pSPIx, DISABLE);

    /*
     * Configure device mode.
     *
     * MSTR = 1: Master
     * MSTR = 0: Slave
     */

    if (pSPIHandle->SPIConfig.SPI_DeviceMode ==
        SPI_DEVICE_MODE_MASTER)
    {
        cr1 |= SPI_CR1_MSTR;
    }

    /* Configure clock polarity. */

    if (pSPIHandle->SPIConfig.SPI_CPOL == SPI_CPOL_HIGH)
    {
        cr1 |= SPI_CR1_CPOL;
    }

    /* Configure clock phase. */

    if (pSPIHandle->SPIConfig.SPI_CPHA == SPI_CPHA_HIGH)
    {
        cr1 |= SPI_CR1_CPHA;
    }

    /* Configure serial clock prescaler. */

    cr1 |=
        ((uint32_t)pSPIHandle->SPIConfig.SPI_SclkSpeed << 3U);

    /* Configure data frame format. */

    if (pSPIHandle->SPIConfig.SPI_DFF == SPI_DFF_16BITS)
    {
        cr1 |= SPI_CR1_DFF;
    }

    /* Configure bus mode. */

    if (pSPIHandle->SPIConfig.SPI_BusConfig ==
        SPI_BUS_CONFIG_HD)
    {
        /*
         * Half-duplex, one-wire mode.
         * Initially configure the line for transmission.
         */
        cr1 |= SPI_CR1_BIDIMODE;
        cr1 |= SPI_CR1_BIDIOE;
    }
    else if (pSPIHandle->SPIConfig.SPI_BusConfig ==
             SPI_BUS_CONFIG_SIMPLEX_RX)
    {
        /*
         * Receive-only, two-line mode.
         */
        cr1 |= SPI_CR1_RXONLY;
    }

    /* Configure software slave management. */

    if (pSPIHandle->SPIConfig.SPI_SSM == SPI_SSM_EN)
    {
        cr1 |= SPI_CR1_SSM;
        cr1 |= SPI_CR1_SSI;
    }

    /*
     * Disable hardware I2S mode.
     * I2SMOD is bit 11 of I2SCFGR.
     */
    pSPIx->I2SCFGR &= ~(1U << 11);

    /* Apply CR1 configuration. */

    pSPIx->CR1 = cr1;

    /*
     * Use default CR2 configuration:
     * no interrupts, no DMA, no hardware NSS output.
     */
    pSPIx->CR2 = 0U;

    /* Enable SPI. */

    SPI_PeripheralControl(pSPIx, ENABLE);
}


/* =========================================================
 * Deinitialize SPI
 * ========================================================= */

void SPI_DeInit(SPI_RegDef_t *pSPIx)
{
    if (pSPIx == SPI1)
    {
        RCC->APB2RSTR |= (1U << 12);
        RCC->APB2RSTR &= ~(1U << 12);
    }
    else if (pSPIx == SPI2)
    {
        RCC->APB1RSTR |= (1U << 14);
        RCC->APB1RSTR &= ~(1U << 14);
    }
    else if (pSPIx == SPI3)
    {
        RCC->APB1RSTR |= (1U << 15);
        RCC->APB1RSTR &= ~(1U << 15);
    }
    else if (pSPIx == SPI4)
    {
        RCC->APB2RSTR |= (1U << 13);
        RCC->APB2RSTR &= ~(1U << 13);
    }

}


/* =========================================================
 * Write one SPI frame
 * ========================================================= */

static uint8_t SPI_WriteFrame(
    SPI_Handle_t *pSPIHandle,
    uint16_t data
)
{
    SPI_RegDef_t *pSPIx = pSPIHandle->pSPIx;
    uint8_t status;

    status = SPI_WaitForFlag(
        pSPIx,
        SPI_FLAG_TXE,
        SET
    );

    if (status != SPI_STATUS_OK)
    {
        return status;
    }

    if (pSPIHandle->SPIConfig.SPI_DFF == SPI_DFF_16BITS)
    {
        *(volatile uint16_t *)&pSPIx->DR = data;
    }
    else
    {
    	*(volatile uint8_t *)&pSPIx->DR = (uint8_t)data;
    }

    return SPI_STATUS_OK;
}


/* =========================================================
 * Read one SPI frame
 * ========================================================= */

static uint16_t SPI_ReadFrame(SPI_Handle_t *pSPIHandle)
{
    SPI_RegDef_t *pSPIx = pSPIHandle->pSPIx;

    if (pSPIHandle->SPIConfig.SPI_DFF == SPI_DFF_16BITS)
    {
        return *(volatile uint16_t *)&pSPIx->DR;
    }

    return *(volatile uint8_t *)&pSPIx->DR;
}


/* =========================================================
 * Send data
 *
 * Len is the number of bytes.
 * In 16-bit mode, Len must be even.
 *
 * This implementation uses full-duplex transfers and
 * reads the simultaneously received data to avoid OVR.
 * ========================================================= */

uint8_t SPI_SendData(
    SPI_Handle_t *pSPIHandle,
    uint8_t *pTxBuffer,
    uint32_t Len
)
{
    SPI_RegDef_t *pSPIx;
    uint32_t i = 0U;
    uint16_t frame;
    uint8_t status;

    if ((pSPIHandle == 0) ||
        (pTxBuffer == 0) ||
        (Len == 0U))
    {
        return SPI_STATUS_ERROR;
    }

    pSPIx = pSPIHandle->pSPIx;

    if ((pSPIx == 0) ||
        (SPI_IsValidPeripheral(pSPIx) == 0U))
    {
        return SPI_STATUS_ERROR;
    }

    if (pSPIHandle->SPIConfig.SPI_BusConfig !=
        SPI_BUS_CONFIG_FD)
    {
        return SPI_STATUS_ERROR;
    }

    if ((pSPIHandle->SPIConfig.SPI_DFF == SPI_DFF_16BITS) &&
        ((Len & 1U) != 0U))
    {
        return SPI_STATUS_ERROR;
    }

    while (i < Len)
    {
        if (pSPIHandle->SPIConfig.SPI_DFF == SPI_DFF_16BITS)
        {
            frame = (uint16_t)pTxBuffer[i] |
                    ((uint16_t)pTxBuffer[i + 1U] << 8U);

            i += 2U;
        }
        else
        {
            frame = pTxBuffer[i];

            i++;
        }

        status = SPI_WriteFrame(pSPIHandle, frame);

        if (status != SPI_STATUS_OK)
        {
            return status;
        }

        status = SPI_WaitForFlag(
            pSPIx,
            SPI_FLAG_RXNE,
            SET
        );

        if (status != SPI_STATUS_OK)
        {
            return status;
        }

        /* Discard simultaneously received data. */
        (void)SPI_ReadFrame(pSPIHandle);
    }

    /* Wait until the final frame leaves the SPI bus. */

    status = SPI_WaitUntilNotBusy(pSPIx);

    return status;
}


/* =========================================================
 * Receive data
 *
 * In master full-duplex mode, dummy frames generate SCK.
 * Len is the number of bytes.
 * In 16-bit mode, Len must be even.
 * ========================================================= */

uint8_t SPI_ReceiveData(
    SPI_Handle_t *pSPIHandle,
    uint8_t *pRxBuffer,
    uint32_t Len
)
{
    SPI_RegDef_t *pSPIx;
    uint32_t i = 0U;
    uint16_t received;
    uint8_t status;

    if ((pSPIHandle == 0) ||
        (pRxBuffer == 0) ||
        (Len == 0U))
    {
        return SPI_STATUS_ERROR;
    }

    pSPIx = pSPIHandle->pSPIx;

    if ((pSPIx == 0) ||
        (SPI_IsValidPeripheral(pSPIx) == 0U))
    {
        return SPI_STATUS_ERROR;
    }

    /*
     * This receive function uses dummy writes and is intended
     * for full-duplex operation.
     */

    if (pSPIHandle->SPIConfig.SPI_BusConfig !=
        SPI_BUS_CONFIG_FD)
    {
        return SPI_STATUS_ERROR;
    }

    if ((pSPIHandle->SPIConfig.SPI_DFF == SPI_DFF_16BITS) &&
        ((Len & 1U) != 0U))
    {
        return SPI_STATUS_ERROR;
    }

    while (i < Len)
    {
        /* Dummy write generates clock pulses. */

        status = SPI_WriteFrame(pSPIHandle, 0xFFFFU);

        if (status != SPI_STATUS_OK)
        {
            return status;
        }

        status = SPI_WaitForFlag(
            pSPIx,
            SPI_FLAG_RXNE,
            SET
        );

        if (status != SPI_STATUS_OK)
        {
            return status;
        }

        received = SPI_ReadFrame(pSPIHandle);

        if (pSPIHandle->SPIConfig.SPI_DFF == SPI_DFF_16BITS)
        {
            pRxBuffer[i] = (uint8_t)(received & 0xFFU);
            pRxBuffer[i + 1U] = (uint8_t)(received >> 8U);

            i += 2U;
        }
        else
        {
            pRxBuffer[i] = (uint8_t)received;

            i++;
        }
    }

    status = SPI_WaitUntilNotBusy(pSPIx);

    return status;
}


/* =========================================================
 * Full-duplex transmit and receive
 *
 * Len is the number of bytes.
 * In 16-bit mode, Len must be even.
 * ========================================================= */

uint8_t SPI_TransmitReceive(
    SPI_Handle_t *pSPIHandle,
    uint8_t *pTxBuffer,
    uint8_t *pRxBuffer,
    uint32_t Len
)
{
    SPI_RegDef_t *pSPIx;
    uint32_t i = 0U;
    uint16_t frame;
    uint16_t received;
    uint8_t status;

    if ((pSPIHandle == 0) ||
        (pTxBuffer == 0) ||
        (pRxBuffer == 0) ||
        (Len == 0U))
    {
        return SPI_STATUS_ERROR;
    }

    pSPIx = pSPIHandle->pSPIx;

    if ((pSPIx == 0) ||
        (SPI_IsValidPeripheral(pSPIx) == 0U))
    {
        return SPI_STATUS_ERROR;
    }

    if (pSPIHandle->SPIConfig.SPI_BusConfig !=
        SPI_BUS_CONFIG_FD)
    {
        return SPI_STATUS_ERROR;
    }

    if ((pSPIHandle->SPIConfig.SPI_DFF == SPI_DFF_16BITS) &&
        ((Len & 1U) != 0U))
    {
        return SPI_STATUS_ERROR;
    }

    while (i < Len)
    {
        if (pSPIHandle->SPIConfig.SPI_DFF == SPI_DFF_16BITS)
        {
            frame = (uint16_t)pTxBuffer[i] |
                    ((uint16_t)pTxBuffer[i + 1U] << 8U);
        }
        else
        {
            frame = pTxBuffer[i];
        }

        status = SPI_WriteFrame(pSPIHandle, frame);

        if (status != SPI_STATUS_OK)
        {
            return status;
        }

        status = SPI_WaitForFlag(
            pSPIx,
            SPI_FLAG_RXNE,
            SET
        );

        if (status != SPI_STATUS_OK)
        {
            return status;
        }

        received = SPI_ReadFrame(pSPIHandle);

        if (pSPIHandle->SPIConfig.SPI_DFF == SPI_DFF_16BITS)
        {
            pRxBuffer[i] = (uint8_t)(received & 0xFFU);
            pRxBuffer[i + 1U] = (uint8_t)(received >> 8U);

            i += 2U;
        }
        else
        {
            pRxBuffer[i] = (uint8_t)received;

            i++;
        }
    }

    return SPI_WaitUntilNotBusy(pSPIx);
}


/* =========================================================
 * Clear overrun flag
 *
 * STM32F4 procedure:
 * 1. Read DR
 * 2. Read SR
 * ========================================================= */

void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx)
{
    volatile uint32_t dummyRead;

    if (pSPIx == 0)
    {
        return;
    }

    dummyRead = pSPIx->DR;
    dummyRead = pSPIx->SR;

    (void)dummyRead;
}
