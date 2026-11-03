/*
 * stm32f446xx_usart_driver.h
 *
 * Register-level USART/UART driver for STM32F446xx
 */

#ifndef INC_STM32F446XX_USART_DRIVER_H_
#define INC_STM32F446XX_USART_DRIVER_H_

#include "stm32f446xx.h"


/* =========================================================
 * USART/UART base addresses
 * Define only if they are not already defined in the
 * main STM32F446xx header.
 * ========================================================= */

#ifndef USART1_BASEADDR
#define USART1_BASEADDR    0x40011000UL
#endif

#ifndef USART2_BASEADDR
#define USART2_BASEADDR    0x40004400UL
#endif

#ifndef USART3_BASEADDR
#define USART3_BASEADDR    0x40004800UL
#endif

#ifndef UART4_BASEADDR
#define UART4_BASEADDR     0x40004C00UL
#endif

#ifndef UART5_BASEADDR
#define UART5_BASEADDR     0x40005000UL
#endif

#ifndef USART6_BASEADDR
#define USART6_BASEADDR    0x40011400UL
#endif

#ifndef UART7_BASEADDR
#define UART7_BASEADDR     0x40007800UL
#endif

#ifndef UART8_BASEADDR
#define UART8_BASEADDR     0x40007C00UL
#endif


/* =========================================================
 * USART register definition
 * ========================================================= */



/* Peripheral pointers */




/* =========================================================
 * Baud rates
 * ========================================================= */

#define USART_STD_BAUD_1200       1200U
#define USART_STD_BAUD_2400       2400U
#define USART_STD_BAUD_4800       4800U
#define USART_STD_BAUD_9600       9600U
#define USART_STD_BAUD_19200      19200U
#define USART_STD_BAUD_38400      38400U
#define USART_STD_BAUD_57600      57600U
#define USART_STD_BAUD_115200     115200U
#define USART_STD_BAUD_230400     230400U
#define USART_STD_BAUD_460800     460800U


/*
 * Peripheral input clock in Hz.
 *
 * These defaults are examples, not automatic clock detection.
 * Change them to match your RCC configuration.
 */

#ifndef USART1_PCLK_FREQ
#define USART1_PCLK_FREQ    16000000U
#endif

#ifndef USART2_PCLK_FREQ
#define USART2_PCLK_FREQ    16000000U
#endif

#ifndef USART3_PCLK_FREQ
#define USART3_PCLK_FREQ    16000000U
#endif

#ifndef UART4_PCLK_FREQ
#define UART4_PCLK_FREQ     16000000U
#endif

#ifndef UART5_PCLK_FREQ
#define UART5_PCLK_FREQ     16000000U
#endif

#ifndef USART6_PCLK_FREQ
#define USART6_PCLK_FREQ    16000000U
#endif

#ifndef UART7_PCLK_FREQ
#define UART7_PCLK_FREQ     16000000U
#endif

#ifndef UART8_PCLK_FREQ
#define UART8_PCLK_FREQ     16000000U
#endif


/* =========================================================
 * USART mode
 * ========================================================= */

#define USART_MODE_ONLY_TX       0U
#define USART_MODE_ONLY_RX       1U
#define USART_MODE_TXRX          2U


/* Stop bits: CR2 STOP field encodings */

#define USART_STOPBITS_1         0U
#define USART_STOPBITS_0_5       1U
#define USART_STOPBITS_2         2U
#define USART_STOPBITS_1_5       3U


/* Parity */

#define USART_PARITY_DISABLE     0U
#define USART_PARITY_EN_EVEN     1U
#define USART_PARITY_EN_ODD      2U


/* Hardware flow control */

#define USART_HW_FLOW_CTRL_NONE  0U
#define USART_HW_FLOW_CTRL_CTS   1U
#define USART_HW_FLOW_CTRL_RTS   2U
#define USART_HW_FLOW_CTRL_CTS_RTS 3U


/* =========================================================
 * USART SR flags
 * ========================================================= */

#define USART_FLAG_PE       (1U << 0)
#define USART_FLAG_FE       (1U << 1)
#define USART_FLAG_NE       (1U << 2)
#define USART_FLAG_ORE      (1U << 3)
#define USART_FLAG_IDLE     (1U << 4)
#define USART_FLAG_RXNE     (1U << 5)
#define USART_FLAG_TC       (1U << 6)
#define USART_FLAG_TXE      (1U << 7)
#define USART_FLAG_LBD      (1U << 8)
#define USART_FLAG_CTS      (1U << 9)


/* Transfer status */

#define USART_STATUS_OK        0U
#define USART_STATUS_ERROR     1U
#define USART_STATUS_TIMEOUT   2U


/* =========================================================
 * Configuration structure
 * ========================================================= */

typedef struct
{
    uint32_t USART_Baud;

    uint8_t USART_Mode;

    uint8_t USART_NoOfStopBits;

    uint8_t USART_ParityControl;

    uint8_t USART_HWFlowControl;

} USART_Config_t;


/* =========================================================
 * Handle structure
 * ========================================================= */

typedef struct
{
    USART_RegDef_t *pUSARTx;

    USART_Config_t USART_Config;

} USART_Handle_t;


/* =========================================================
 * Driver APIs
 * ========================================================= */

/* Clock control */

void USART_PeriClockControl(
    USART_RegDef_t *pUSARTx,
    uint8_t EnorDi
);


/* Initialization */

void USART_Init(USART_Handle_t *pUSARTHandle);

void USART_DeInit(USART_RegDef_t *pUSARTx);


/* Peripheral enable/disable */

void USART_PeripheralControl(
    USART_RegDef_t *pUSARTx,
    uint8_t EnorDi
);


/* Flag checking */

uint8_t USART_GetFlagStatus(
    USART_RegDef_t *pUSARTx,
    uint32_t FlagName
);


/* Data transfer */

uint8_t USART_SendData(
    USART_Handle_t *pUSARTHandle,
    uint8_t *pTxBuffer,
    uint32_t Len
);

uint8_t USART_ReceiveData(
    USART_Handle_t *pUSARTHandle,
    uint8_t *pRxBuffer,
    uint32_t Len
);


/* Single-character transmit */

uint8_t USART_SendByte(
    USART_Handle_t *pUSARTHandle,
    uint8_t data
);


/* Single-character receive */

uint8_t USART_ReceiveByte(
    USART_Handle_t *pUSARTHandle,
    uint8_t *data
);

#endif /* INC_STM32F446XX_USART_DRIVER_H_ */
