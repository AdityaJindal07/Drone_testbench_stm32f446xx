/*
 * stm32f446xx_i2c_driver.h
 *
 * Register-level I2C driver for STM32F446xx
 */

#ifndef INC_STM32F446XX_I2C_DRIVER_H_
#define INC_STM32F446XX_I2C_DRIVER_H_

#include "stm32f446xx.h"

/*
 * I2C Register Definition
 *
 * Register offsets:
 * CR1   : 0x00
 * CR2   : 0x04
 * OAR1  : 0x08
 * OAR2  : 0x0C
 * DR    : 0x10
 * SR1   : 0x14
 * SR2   : 0x18
 * CCR   : 0x1C
 * TRISE : 0x20
 * FLTR  : 0x24
 */




/* I2C operating modes */

#define I2C_SCL_SPEED_SM       100000U
#define I2C_SCL_SPEED_FM       400000U

#define I2C_ACK_ENABLE         1U
#define I2C_ACK_DISABLE        0U

#define I2C_FM_DUTY_2          0U
#define I2C_FM_DUTY_16_9       1U


/*
 * APB1 clock frequency in Hz.
 *
 * Change this value if your RCC clock configuration
 * uses a different APB1 clock frequency.
 */

#ifndef I2C_PCLK1_FREQ
#define I2C_PCLK1_FREQ         16000000U
#endif


/* I2C CR1 register bit positions */

#define I2C_CR1_PE             (1U << 0)
#define I2C_CR1_START          (1U << 8)
#define I2C_CR1_STOP           (1U << 9)
#define I2C_CR1_ACK            (1U << 10)


/* I2C SR1 register flags */

#define I2C_FLAG_SB            (1U << 0)
#define I2C_FLAG_ADDR          (1U << 1)
#define I2C_FLAG_BTF           (1U << 2)
#define I2C_FLAG_STOPF         (1U << 4)
#define I2C_FLAG_RXNE          (1U << 6)
#define I2C_FLAG_TXE           (1U << 7)

#define I2C_FLAG_BERR          (1U << 8)
#define I2C_FLAG_ARLO           (1U << 9)
#define I2C_FLAG_AF             (1U << 10)
#define I2C_FLAG_OVR            (1U << 11)
#define I2C_FLAG_TIMEOUT        (1U << 14)


/* Transfer status */

#define I2C_STATUS_OK           0U
#define I2C_STATUS_ERROR        1U
#define I2C_STATUS_TIMEOUT      2U
#define I2C_STATUS_NACK         3U


/* Configuration structure */

typedef struct
{
    uint32_t I2C_SCLSpeed;
    uint8_t  I2C_ACKControl;
    uint8_t  I2C_FMDutyCycle;

} I2C_Config_t;


/* Handle structure */

typedef struct
{
    I2C_RegDef_t *pI2Cx;

    I2C_Config_t I2C_Config;

    uint8_t *pTxBuffer;
    uint8_t *pRxBuffer;

    uint32_t TxLen;
    uint32_t RxLen;

} I2C_Handle_t;


/* Peripheral clock control */

void I2C_PeriClockControl(
    I2C_RegDef_t *pI2Cx,
    uint8_t EnorDi
);


/* Initialization and deinitialization */

void I2C_Init(I2C_Handle_t *pI2CHandle);

void I2C_DeInit(I2C_RegDef_t *pI2Cx);


/* Peripheral control */

void I2C_PeripheralControl(
    I2C_RegDef_t *pI2Cx,
    uint8_t EnorDi
);


/* ACK control */

void I2C_ManageAcking(
    I2C_RegDef_t *pI2Cx,
    uint8_t EnorDi
);


/* START and STOP conditions */

uint8_t I2C_GenerateStartCondition(
    I2C_RegDef_t *pI2Cx
);

void I2C_GenerateStopCondition(
    I2C_RegDef_t *pI2Cx
);


/* Address phase */

uint8_t I2C_ExecuteAddressPhaseWrite(
    I2C_RegDef_t *pI2Cx,
    uint8_t SlaveAddr
);

uint8_t I2C_ExecuteAddressPhaseRead(
    I2C_RegDef_t *pI2Cx,
    uint8_t SlaveAddr
);


/* Master data transfer */

uint8_t I2C_MasterSendData(
    I2C_Handle_t *pI2CHandle,
    uint8_t *pTxBuffer,
    uint32_t Len,
    uint8_t SlaveAddr
);

uint8_t I2C_MasterReceiveData(
    I2C_Handle_t *pI2CHandle,
    uint8_t *pRxBuffer,
    uint32_t Len,
    uint8_t SlaveAddr
);


/* Flag status */

uint8_t I2C_GetFlagStatus(
    I2C_RegDef_t *pI2Cx,
    uint32_t FlagName
);

#endif /* INC_STM32F446XX_I2C_DRIVER_H_ */
