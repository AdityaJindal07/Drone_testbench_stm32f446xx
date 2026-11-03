/*
 * stm32f446xx_uart_driver.c
 *
 *  Created on: Oct 15, 2026
 *      Author: aditya-jindal007
 */


/*
 * stm32f446xx_usart_driver.c
 *
 * Register-level USART/UART driver for STM32F446xx
 */

#include "stm32f446xx_uart_driver.h"


#define USART_TIMEOUT_VALUE    1000000U

#define USART_CR1_RE            (1U << 2)
#define USART_CR1_TE            (1U << 3)
#define USART_CR1_PCE           (1U << 10)
#define USART_CR1_PS            (1U << 9)
#define USART_CR1_UE            (1U << 13)

#define USART_CR2_STOP_MASK     (3U << 12)

#define USART_CR3_RTSE          (1U << 8)
#define USART_CR3_CTSE          (1U << 9)

#define USART_ERROR_FLAGS       (USART_FLAG_PE | \
                                 USART_FLAG_FE | \
                                 USART_FLAG_NE | \
                                 USART_FLAG_ORE)


/* =========================================================
 * Private function declarations
 * ========================================================= */

static uint32_t USART_GetPCLKFrequency(
    USART_RegDef_t *pUSARTx
);

static uint8_t USART_IsValidPeripheral(
    USART_RegDef_t *pUSARTx
);

static uint8_t USART_WaitForFlag(
    USART_RegDef_t *pUSARTx,
    uint32_t FlagName
);

static void USART_ClearErrorFlags(
    USART_RegDef_t *pUSARTx
);


/* =========================================================
 * Check peripheral pointer
 * ========================================================= */

static uint8_t USART_IsValidPeripheral(
    USART_RegDef_t *pUSARTx
)
{
    if ((pUSARTx == USART1) ||
        (pUSARTx == USART2) ||
        (pUSARTx == USART3) ||
        (pUSARTx == UART4)  ||
        (pUSARTx == UART5)  ||
        (pUSARTx == USART6) ||
        (pUSARTx == UART7)  ||
        (pUSARTx == UART8))
    {
        return 1U;
    }

    return 0U;
}


/* =========================================================
 * Get peripheral input clock frequency
 * ========================================================= */

static uint32_t USART_GetPCLKFrequency(
    USART_RegDef_t *pUSARTx
)
{
    if (pUSARTx == USART1)
    {
        return USART1_PCLK_FREQ;
    }

    if (pUSARTx == USART2)
    {
        return USART2_PCLK_FREQ;
    }

    if (pUSARTx == USART3)
    {
        return USART3_PCLK_FREQ;
    }

    if (pUSARTx == UART4)
    {
        return UART4_PCLK_FREQ;
    }

    if (pUSARTx == UART5)
    {
        return UART5_PCLK_FREQ;
    }

    if (pUSARTx == USART6)
    {
        return USART6_PCLK_FREQ;
    }

    if (pUSARTx == UART7)
    {
        return UART7_PCLK_FREQ;
    }

    if (pUSARTx == UART8)
    {
        return UART8_PCLK_FREQ;
    }

    return 0U;
}


/* =========================================================
 * Peripheral clock control
 * ========================================================= */

void USART_PeriClockControl(
    USART_RegDef_t *pUSARTx,
    uint8_t EnorDi
)
{
    if (EnorDi != ENABLE)
    {
        return;
    }

    if (pUSARTx == USART1)
    {
        USART1_PCLK_EN();
    }
    else if (pUSARTx == USART2)
    {
        USART2_PCLK_EN();
    }
    else if (pUSARTx == USART3)
    {
        USART3_PCLK_EN();
    }
    else if (pUSARTx == UART4)
    {
        UART4_PCLK_EN();
    }
    else if (pUSARTx == UART5)
    {
        UART5_PCLK_EN();
    }
    else if (pUSARTx == USART6)
    {
        USART6_PCLK_EN();
    }

}


/* =========================================================
 * Peripheral enable / disable
 * ========================================================= */

void USART_PeripheralControl(
    USART_RegDef_t *pUSARTx,
    uint8_t EnorDi
)
{
    if (pUSARTx == 0)
    {
        return;
    }

    if (EnorDi == ENABLE)
    {
        pUSARTx->CR1 |= USART_CR1_UE;
    }
    else
    {
        pUSARTx->CR1 &= ~USART_CR1_UE;
    }
}


/* =========================================================
 * Initialize USART/UART
 *
 * This driver uses:
 * - 8-bit data
 * - Oversampling by 16
 * - Configurable baud rate
 * - Configurable parity and stop bits
 * ========================================================= */

void USART_Init(USART_Handle_t *pUSARTHandle)
{
    USART_RegDef_t *pUSARTx;
    uint32_t pclk;
    uint32_t baud;
    uint32_t brr;
    uint32_t cr1 = 0U;
    uint32_t cr2 = 0U;
    uint32_t cr3 = 0U;

    if (pUSARTHandle == 0)
    {
        return;
    }

    pUSARTx = pUSARTHandle->pUSARTx;

    if (USART_IsValidPeripheral(pUSARTx) == 0U)
    {
        return;
    }

    baud = pUSARTHandle->USART_Config.USART_Baud;
    pclk = USART_GetPCLKFrequency(pUSARTx);

    if ((baud == 0U) || (pclk == 0U))
    {
        return;
    }

    /* Validate the configuration values. */

    if ((pUSARTHandle->USART_Config.USART_Mode >
         USART_MODE_TXRX) ||
        (pUSARTHandle->USART_Config.USART_NoOfStopBits >
         USART_STOPBITS_1_5) ||
        (pUSARTHandle->USART_Config.USART_ParityControl >
         USART_PARITY_EN_ODD) ||
        (pUSARTHandle->USART_Config.USART_HWFlowControl >
         USART_HW_FLOW_CTRL_CTS_RTS))
    {
        return;
    }

    /* Enable the peripheral clock. */

    USART_PeriClockControl(pUSARTx, ENABLE);

    /* Disable the USART while configuring it. */

    USART_PeripheralControl(pUSARTx, DISABLE);

    /*
     * Baud-rate calculation for oversampling by 16:
     *
     * USARTDIV = PCLK / BaudRate
     *
     * BRR is the rounded integer representation of
     * USARTDIV for oversampling by 16.
     */

    brr = (pclk + (baud / 2U)) / baud;

    if ((brr == 0U) || (brr > 0xFFFFU))
    {
        return;
    }

    pUSARTx->BRR = brr;

    /* -----------------------------------------------------
     * CR1 configuration
     * ----------------------------------------------------- */

    /* Enable transmitter and/or receiver. */

    if ((pUSARTHandle->USART_Config.USART_Mode ==
         USART_MODE_ONLY_TX) ||
        (pUSARTHandle->USART_Config.USART_Mode ==
         USART_MODE_TXRX))
    {
        cr1 |= USART_CR1_TE;
    }

    if ((pUSARTHandle->USART_Config.USART_Mode ==
         USART_MODE_ONLY_RX) ||
        (pUSARTHandle->USART_Config.USART_Mode ==
         USART_MODE_TXRX))
    {
        cr1 |= USART_CR1_RE;
    }

    /* Parity configuration. */

    if (pUSARTHandle->USART_Config.USART_ParityControl ==
        USART_PARITY_EN_EVEN)
    {
        cr1 |= USART_CR1_PCE;
    }
    else if (
        pUSARTHandle->USART_Config.USART_ParityControl ==
        USART_PARITY_EN_ODD)
    {
        cr1 |= USART_CR1_PCE;
        cr1 |= USART_CR1_PS;
    }

    /*
     * M = 0: 8-bit word length.
     * OVER8 = 0: oversampling by 16.
     */

    pUSARTx->CR1 = cr1;

    /* -----------------------------------------------------
     * CR2 configuration: stop bits
     * ----------------------------------------------------- */

    cr2 |=
        ((uint32_t)pUSARTHandle->USART_Config.USART_NoOfStopBits
         << 12U);

    pUSARTx->CR2 = cr2;

    /* -----------------------------------------------------
     * CR3 configuration: hardware flow control
     * ----------------------------------------------------- */

    if ((pUSARTHandle->USART_Config.USART_HWFlowControl ==
         USART_HW_FLOW_CTRL_RTS) ||
        (pUSARTHandle->USART_Config.USART_HWFlowControl ==
         USART_HW_FLOW_CTRL_CTS_RTS))
    {
        cr3 |= USART_CR3_RTSE;
    }

    if ((pUSARTHandle->USART_Config.USART_HWFlowControl ==
         USART_HW_FLOW_CTRL_CTS) ||
        (pUSARTHandle->USART_Config.USART_HWFlowControl ==
         USART_HW_FLOW_CTRL_CTS_RTS))
    {
        cr3 |= USART_CR3_CTSE;
    }

    pUSARTx->CR3 = cr3;

    /* Enable USART after configuration. */

    USART_PeripheralControl(pUSARTx, ENABLE);
}


/* =========================================================
 * Deinitialize USART/UART
 * ========================================================= */

void USART_DeInit(USART_RegDef_t *pUSARTx)
{
    if (pUSARTx == USART1)
    {
        RCC->APB2RSTR |= (1U << 4);
        RCC->APB2RSTR &= ~(1U << 4);
    }
    else if (pUSARTx == USART6)
    {
        RCC->APB2RSTR |= (1U << 5);
        RCC->APB2RSTR &= ~(1U << 5);
    }
    else if (pUSARTx == USART2)
    {
        RCC->APB1RSTR |= (1U << 17);
        RCC->APB1RSTR &= ~(1U << 17);
    }
    else if (pUSARTx == USART3)
    {
        RCC->APB1RSTR |= (1U << 18);
        RCC->APB1RSTR &= ~(1U << 18);
    }
    else if (pUSARTx == UART4)
    {
        RCC->APB1RSTR |= (1U << 19);
        RCC->APB1RSTR &= ~(1U << 19);
    }
    else if (pUSARTx == UART5)
    {
        RCC->APB1RSTR |= (1U << 20);
        RCC->APB1RSTR &= ~(1U << 20);
    }
    else if (pUSARTx == UART7)
    {
        RCC->APB1RSTR |= (1U << 30);
        RCC->APB1RSTR &= ~(1U << 30);
    }
    else if (pUSARTx == UART8)
    {
        RCC->APB1RSTR |= (1U << 31);
        RCC->APB1RSTR &= ~(1U << 31);
    }
}


/* =========================================================
 * Get flag status
 * ========================================================= */

uint8_t USART_GetFlagStatus(
    USART_RegDef_t *pUSARTx,
    uint32_t FlagName
)
{
    if (pUSARTx == 0)
    {
        return RESET;
    }

    if ((pUSARTx->SR & FlagName) != 0U)
    {
        return SET;
    }

    return RESET;
}


/* =========================================================
 * Wait for a flag with timeout
 * ========================================================= */

static uint8_t USART_WaitForFlag(
    USART_RegDef_t *pUSARTx,
    uint32_t FlagName
)
{
    uint32_t timeout = USART_TIMEOUT_VALUE;

    while (timeout > 0U)
    {
        if ((pUSARTx->SR & USART_ERROR_FLAGS) != 0U)
        {
            return USART_STATUS_ERROR;
        }

        if ((pUSARTx->SR & FlagName) != 0U)
        {
            return USART_STATUS_OK;
        }

        timeout--;
    }

    return USART_STATUS_TIMEOUT;
}


/* =========================================================
 * Clear USART error flags
 *
 * On STM32F4, reading SR followed by DR clears the
 * PE, FE, NE and ORE error flags.
 * ========================================================= */

static void USART_ClearErrorFlags(
    USART_RegDef_t *pUSARTx
)
{
    volatile uint32_t dummyRead;

    if ((pUSARTx->SR & USART_ERROR_FLAGS) != 0U)
    {
        dummyRead = pUSARTx->SR;
        dummyRead = pUSARTx->DR;

        (void)dummyRead;
    }
}


/* =========================================================
 * Send one byte
 * ========================================================= */

uint8_t USART_SendByte(
    USART_Handle_t *pUSARTHandle,
    uint8_t data
)
{
    uint8_t status;

    if ((pUSARTHandle == 0) ||
        (pUSARTHandle->pUSARTx == 0))
    {
        return USART_STATUS_ERROR;
    }

    /* Wait until the data register is empty. */

    status = USART_WaitForFlag(
        pUSARTHandle->pUSARTx,
        USART_FLAG_TXE
    );

    if (status != USART_STATUS_OK)
    {
        return status;
    }

    /* Write data to the data register. */

    pUSARTHandle->pUSARTx->DR = data;

    return USART_STATUS_OK;
}


/* =========================================================
 * Send a buffer
 * ========================================================= */

uint8_t USART_SendData(
    USART_Handle_t *pUSARTHandle,
    uint8_t *pTxBuffer,
    uint32_t Len
)
{
    uint32_t i;
    uint8_t status;

    if ((pUSARTHandle == 0) ||
        (pUSARTHandle->pUSARTx == 0) ||
        (pTxBuffer == 0) ||
        (Len == 0U))
    {
        return USART_STATUS_ERROR;
    }

    for (i = 0U; i < Len; i++)
    {
        status = USART_SendByte(
            pUSARTHandle,
            pTxBuffer[i]
        );

        if (status != USART_STATUS_OK)
        {
            return status;
        }
    }

    /*
     * TXE means the data register is empty.
     * TC means the last frame has finished transmitting.
     */

    status = USART_WaitForFlag(
        pUSARTHandle->pUSARTx,
        USART_FLAG_TC
    );

    return status;
}


/* =========================================================
 * Receive one byte
 * ========================================================= */

uint8_t USART_ReceiveByte(
    USART_Handle_t *pUSARTHandle,
    uint8_t *data
)
{
    uint8_t status;

    if ((pUSARTHandle == 0) ||
        (pUSARTHandle->pUSARTx == 0) ||
        (data == 0))
    {
        return USART_STATUS_ERROR;
    }

    status = USART_WaitForFlag(
        pUSARTHandle->pUSARTx,
        USART_FLAG_RXNE
    );

    if (status != USART_STATUS_OK)
    {
        USART_ClearErrorFlags(pUSARTHandle->pUSARTx);
        return status;
    }

    /* Read the received byte. */

    *data = (uint8_t)pUSARTHandle->pUSARTx->DR;

    return USART_STATUS_OK;
}


/* =========================================================
 * Receive a buffer
 * ========================================================= */

uint8_t USART_ReceiveData(
    USART_Handle_t *pUSARTHandle,
    uint8_t *pRxBuffer,
    uint32_t Len
)
{
    uint32_t i;
    uint8_t status;

    if ((pUSARTHandle == 0) ||
        (pUSARTHandle->pUSARTx == 0) ||
        (pRxBuffer == 0) ||
        (Len == 0U))
    {
        return USART_STATUS_ERROR;
    }

    for (i = 0U; i < Len; i++)
    {
        status = USART_ReceiveByte(
            pUSARTHandle,
            &pRxBuffer[i]
        );

        if (status != USART_STATUS_OK)
        {
            return status;
        }
    }

    return USART_STATUS_OK;
}
