/*
 * stm32f446xx_spi_driver.h
 *
 *  Created on: Oct 15, 2026
 *      Author: aditya-jindal007
 */

/*
 * stm32f446xx_spi_driver.h
 *
 * Register-level SPI driver for STM32F446xx
 */

#ifndef INC_STM32F446XX_SPI_DRIVER_H_
#define INC_STM32F446XX_SPI_DRIVER_H_

#include "stm32f446xx.h"


/* =========================================================
 * SPI base addresses
 * ========================================================= */

#ifndef SPI1_BASEADDR
#define SPI1_BASEADDR    0x40013000UL
#endif

#ifndef SPI2_BASEADDR
#define SPI2_BASEADDR    0x40003800UL
#endif

#ifndef SPI3_BASEADDR
#define SPI3_BASEADDR    0x40003C00UL
#endif

#ifndef SPI4_BASEADDR
#define SPI4_BASEADDR    0x40013400UL
#endif


/* =========================================================
 * SPI operating modes
 * ========================================================= */

#define SPI_DEVICE_MODE_MASTER    1U
#define SPI_DEVICE_MODE_SLAVE     0U


/* SPI bus configurations */

#define SPI_BUS_CONFIG_FD         1U
#define SPI_BUS_CONFIG_HD         2U
#define SPI_BUS_CONFIG_SIMPLEX_RX 3U


/* SPI clock speeds: BR field encodings */

#define SPI_SCLK_SPEED_DIV2       0U
#define SPI_SCLK_SPEED_DIV4       1U
#define SPI_SCLK_SPEED_DIV8       2U
#define SPI_SCLK_SPEED_DIV16      3U
#define SPI_SCLK_SPEED_DIV32      4U
#define SPI_SCLK_SPEED_DIV64      5U
#define SPI_SCLK_SPEED_DIV128     6U
#define SPI_SCLK_SPEED_DIV256     7U


/* Clock polarity */

#define SPI_CPOL_LOW              0U
#define SPI_CPOL_HIGH             1U


/* Clock phase */

#define SPI_CPHA_LOW              0U
#define SPI_CPHA_HIGH             1U


/* Data frame format */

#define SPI_DFF_8BITS             0U
#define SPI_DFF_16BITS            1U


/* Software slave management */

#define SPI_SSM_DI                0U
#define SPI_SSM_EN                1U


/* SPI status flags: SR register */

#define SPI_FLAG_RXNE             (1U << 0)
#define SPI_FLAG_TXE              (1U << 1)
#define SPI_FLAG_CHSIDE            (1U << 2)
#define SPI_FLAG_UDR              (1U << 3)
#define SPI_FLAG_CRCERR           (1U << 4)
#define SPI_FLAG_MODF             (1U << 5)
#define SPI_FLAG_OVR              (1U << 6)
#define SPI_FLAG_BSY               (1U << 7)
#define SPI_FLAG_FRE               (1U << 8)


/* SPI transfer status */

#define SPI_STATUS_OK             0U
#define SPI_STATUS_ERROR          1U
#define SPI_STATUS_TIMEOUT        2U


/* =========================================================
 * SPI configuration structure
 * ========================================================= */

typedef struct
{
    uint8_t SPI_DeviceMode;

    uint8_t SPI_BusConfig;

    uint8_t SPI_SclkSpeed;

    uint8_t SPI_CPOL;

    uint8_t SPI_CPHA;

    uint8_t SPI_DFF;

    uint8_t SPI_SSM;

} SPI_Config_t;


/* =========================================================
 * SPI handle structure
 * ========================================================= */

typedef struct
{
    SPI_RegDef_t *pSPIx;

    SPI_Config_t SPIConfig;

} SPI_Handle_t;


/* =========================================================
 * SPI driver APIs
 * ========================================================= */

/* Peripheral clock control */

void SPI_PeriClockControl(
    SPI_RegDef_t *pSPIx,
    uint8_t EnorDi
);


/* Initialization */

void SPI_Init(SPI_Handle_t *pSPIHandle);

void SPI_DeInit(SPI_RegDef_t *pSPIx);


/* Peripheral control */

void SPI_PeripheralControl(
    SPI_RegDef_t *pSPIx,
    uint8_t EnorDi
);


/* Software slave management */

void SPI_SSIConfig(
    SPI_RegDef_t *pSPIx,
    uint8_t EnorDi
);


/* Hardware slave output enable */

void SPI_SSOEConfig(
    SPI_RegDef_t *pSPIx,
    uint8_t EnorDi
);


/* Flag checking */

uint8_t SPI_GetFlagStatus(
    SPI_RegDef_t *pSPIx,
    uint32_t FlagName
);


/* Data transfer */

uint8_t SPI_SendData(
    SPI_Handle_t *pSPIHandle,
    uint8_t *pTxBuffer,
    uint32_t Len
);

uint8_t SPI_ReceiveData(
    SPI_Handle_t *pSPIHandle,
    uint8_t *pRxBuffer,
    uint32_t Len
);


/* Full-duplex transfer */

uint8_t SPI_TransmitReceive(
    SPI_Handle_t *pSPIHandle,
    uint8_t *pTxBuffer,
    uint8_t *pRxBuffer,
    uint32_t Len
);


/* Clear overrun flag */

void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx);

#endif /* INC_STM32F446XX_SPI_DRIVER_H_ */
