/*
 * stm32f446xx.h
 *
 *  Created on: Sep 26, 2026
 *      Author: aditya-jindal007
 */

#ifndef INC_STM32F446XX_H_
#define INC_STM32F446XX_H_

#include <stdint.h>

#define FLASH_BASADDR	  0x08000000U    		//flash memory address
#define SRAM1_BASEADDR	  0x20000000U	   		//srma1 memory address
#define SRAM2_BASEADDR	  0x2001C000U    		//srma2 memory address
#define ROM				  0x1FFF0000U    		//ROM memory address
#define SRAM 			  SRAM1_BASEADDR

// APB AND AHB BUS PERIPHERAL BASE ADDRESSES

#define PERIPH_BASE       0x40000000UL
#define APB1PERIPH_BASE   PERIPH_BASE
#define APB2PERIPH_BASE   0x40010000UL
#define AHB1PERIPH_BASE   0x40020000UL
#define AHB2PERIPH_BASE   0x50000000UL

// BASE ADDRESSES OF PERIPHERALS HANGING ON "AHB1" BUS==============================

#define GPIOA_BASEADDR      (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOB_BASEADDR      (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOC_BASEADDR      (AHB1PERIPH_BASE + 0x0800UL)
#define GPIOD_BASEADDR      (AHB1PERIPH_BASE + 0x0C00UL)
#define GPIOE_BASEADDR      (AHB1PERIPH_BASE + 0x1000UL)
#define GPIOF_BASEADDR      (AHB1PERIPH_BASE + 0x1400UL)
#define GPIOG_BASEADDR      (AHB1PERIPH_BASE + 0x1800UL)
#define GPIOH_BASEADDR      (AHB1PERIPH_BASE + 0x1C00UL)

#define CRC_BASEADDR        (AHB1PERIPH_BASE + 0x3000UL)
#define RCC_BASEADDR        (AHB1PERIPH_BASE + 0x3800UL)

#define DMA1_BASEADDR       (AHB1PERIPH_BASE + 0x6000UL)
#define DMA2_BASEADDR       (AHB1PERIPH_BASE + 0x6400UL)

#define USB_OTG_HS_BASEADDR (AHB1PERIPH_BASE + 0x40000UL)

// BASE ADDRESSES OF PERIPHERALS HANGING ON "AHB2" BUS================================

#define USB_OTG_FS_BASEADDR  (AHB2PERIPH_BASE + 0x00000UL)
#define DCMI_BASEADDR        (AHB2PERIPH_BASE + 0x50000UL)
#define CRYP_BASEADDR        (AHB2PERIPH_BASE + 0x60000UL)
#define HASH_BASEADDR        (AHB2PERIPH_BASE + 0x64000UL)
#define RNG_BASEADDR         (AHB2PERIPH_BASE + 0x68000UL)

// BASE ADDRESSES OF PERIPHERALS HANGING ON "APB1" BUS================================

#define TIM2_BASEADDR       (APB1PERIPH_BASE + 0x0000UL)
#define TIM3_BASEADDR       (APB1PERIPH_BASE + 0x0400UL)
#define TIM4_BASEADDR       (APB1PERIPH_BASE + 0x0800UL)
#define TIM5_BASEADDR       (APB1PERIPH_BASE + 0x0C00UL)

#define RTC_BASEADDR        (APB1PERIPH_BASE + 0x2800UL)
#define WWDG_BASEADDR       (APB1PERIPH_BASE + 0x2C00UL)
#define IWDG_BASEADDR       (APB1PERIPH_BASE + 0x3000UL)

#define I2S2_BASEADDR       (APB1PERIPH_BASE + 0x3800UL)
#define SPI2_BASEADDR       (APB1PERIPH_BASE + 0x3800UL)

#define SPI3_BASEADDR       (APB1PERIPH_BASE + 0x3C00UL)
#define I2S3_BASEADDR       (APB1PERIPH_BASE + 0x3C00UL)

#define USART2_BASEADDR     (APB1PERIPH_BASE + 0x4400UL)
#define USART3_BASEADDR     (APB1PERIPH_BASE + 0x4800UL)
#define UART4_BASEADDR      (APB1PERIPH_BASE + 0x4C00UL)
#define UART5_BASEADDR      (APB1PERIPH_BASE + 0x5000UL)

#define I2C1_BASEADDR       (APB1PERIPH_BASE + 0x5400UL)
#define I2C2_BASEADDR       (APB1PERIPH_BASE + 0x5800UL)
#define I2C3_BASEADDR       (APB1PERIPH_BASE + 0x5C00UL)

#define PWR_BASEADDR        (APB1PERIPH_BASE + 0x7000UL)
#define DAC_BASEADDR        (APB1PERIPH_BASE + 0x7400UL)

// BASE ADDRESSES OF PERIPHERALS HANGING ON "APB2" BUS==============================

#define TIM1_BASEADDR       (APB2PERIPH_BASE + 0x0000UL)
#define TIM8_BASEADDR       (APB2PERIPH_BASE + 0x0400UL)

#define USART1_BASEADDR     (APB2PERIPH_BASE + 0x1000UL)
#define USART6_BASEADDR     (APB2PERIPH_BASE + 0x1400UL)

#define ADC1_BASEADDR       (APB2PERIPH_BASE + 0x2000UL)
#define ADC2_BASEADDR       (APB2PERIPH_BASE + 0x2100UL)
#define ADC3_BASEADDR       (APB2PERIPH_BASE + 0x2200UL)

#define SDIO_BASEADDR       (APB2PERIPH_BASE + 0x2C00UL)

#define SPI1_BASEADDR       (APB2PERIPH_BASE + 0x3000UL)
#define SPI4_BASEADDR       (APB2PERIPH_BASE + 0x3400UL)

#define SYSCFG_BASEADDR     (APB2PERIPH_BASE + 0x3800UL)
#define EXTI_BASEADDR       (APB2PERIPH_BASE + 0x3C00UL)

#define TIM9_BASEADDR       (APB2PERIPH_BASE + 0x4000UL)
#define TIM10_BASEADDR      (APB2PERIPH_BASE + 0x4400UL)
#define TIM11_BASEADDR      (APB2PERIPH_BASE + 0x4800UL)

// GPIO PERIPHERAL REGISTER DEFINITION STRUCTURE====================================

typedef struct  //for GPIO
{
    volatile uint32_t MODER;       // 0x00 - GPIO port mode register
    volatile uint32_t OTYPER;      // 0x04 - GPIO port output type register
    volatile uint32_t OSPEEDR;     // 0x08 - GPIO port output speed register
    volatile uint32_t PUPDR;       // 0x0C - GPIO port pull-up/pull-down register
    volatile uint32_t IDR;         // 0x10 - GPIO port input data register
    volatile uint32_t ODR;         // 0x14 - GPIO port output data register
    volatile uint32_t BSRR;        // 0x18 - GPIO port bit set/reset register
    volatile uint32_t LCKR;        // 0x1C - GPIO port configuration lock register
    volatile uint32_t AFR[2];      // 0x20 - 0x24 - Alternate function registers (low and high)
} GPIO_RegDef_t;


typedef struct
{
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t OAR1;
    volatile uint32_t OAR2;
    volatile uint32_t DR;
    volatile uint32_t SR1;
    volatile uint32_t SR2;
    volatile uint32_t CCR;
    volatile uint32_t TRISE;
    volatile uint32_t FLTR;

} I2C_RegDef_t;

typedef struct
{
    volatile uint32_t CR1;       /* Control register 1     0x00 */
    volatile uint32_t CR2;       /* Control register 2     0x04 */
    volatile uint32_t SR;        /* Status register        0x08 */
    volatile uint32_t DR;        /* Data register          0x0C */
    volatile uint32_t CRCPR;     /* CRC polynomial         0x10 */
    volatile uint32_t RXCRCR;    /* RX CRC register        0x14 */
    volatile uint32_t TXCRCR;    /* TX CRC register        0x18 */
    volatile uint32_t I2SCFGR;   /* I2S configuration      0x1C */
    volatile uint32_t I2SPR;     /* I2S prescaler          0x20 */

} SPI_RegDef_t;

typedef struct
{
    volatile uint32_t SR;       /* Status register       0x00 */
    volatile uint32_t DR;       /* Data register         0x04 */
    volatile uint32_t BRR;      /* Baud rate register    0x08 */
    volatile uint32_t CR1;      /* Control register 1    0x0C */
    volatile uint32_t CR2;      /* Control register 2    0x10 */
    volatile uint32_t CR3;      /* Control register 3    0x14 */
    volatile uint32_t GTPR;     /* Guard time/prescaler  0x18 */

} USART_RegDef_t;



typedef struct  //for RCC
{
    volatile uint32_t CR;          // 0x00 - Clock control register
    volatile uint32_t PLLCFGR;     // 0x04 - PLL configuration register
    volatile uint32_t CFGR;        // 0x08 - Clock configuration register
    volatile uint32_t CIR;         // 0x0C - Clock interrupt register

    volatile uint32_t AHB1RSTR;    // 0x10 - AHB1 peripheral reset register
    volatile uint32_t AHB2RSTR;    // 0x14 - AHB2 peripheral reset register
    volatile uint32_t AHB3RSTR;    // 0x18 - AHB3 peripheral reset register
    uint32_t RESERVED0;             // 0x1C - Reserved

    volatile uint32_t APB1RSTR;    // 0x20 - APB1 peripheral reset register
    volatile uint32_t APB2RSTR;    // 0x24 - APB2 peripheral reset register
    uint32_t RESERVED1[2];          // 0x28, 0x2C - Reserved

    volatile uint32_t AHB1ENR;     // 0x30 - AHB1 peripheral clock enable register
    volatile uint32_t AHB2ENR;     // 0x34 - AHB2 peripheral clock enable register
    volatile uint32_t AHB3ENR;     // 0x38 - AHB3 peripheral clock enable register
    uint32_t RESERVED2;             // 0x3C - Reserved

    volatile uint32_t APB1ENR;     // 0x40 - APB1 peripheral clock enable register
    volatile uint32_t APB2ENR;     // 0x44 - APB2 peripheral clock enable register
    uint32_t RESERVED3[2];          // 0x48, 0x4C - Reserved

    volatile uint32_t AHB1LPENR;   // 0x50 - AHB1 peripheral clock enable in low-power mode
    volatile uint32_t AHB2LPENR;   // 0x54 - AHB2 peripheral clock enable in low-power mode
    volatile uint32_t AHB3LPENR;   // 0x58 - AHB3 peripheral clock enable in low-power mode
    uint32_t RESERVED4;             // 0x5C - Reserved

    volatile uint32_t APB1LPENR;   // 0x60 - APB1 peripheral clock enable in low-power mode
    volatile uint32_t APB2LPENR;   // 0x64 - APB2 peripheral clock enable in low-power mode
    uint32_t RESERVED5[2];          // 0x68, 0x6C - Reserved

    volatile uint32_t BDCR;         // 0x70 - Backup domain control register
    volatile uint32_t CSR;          // 0x74 - Control/status register
    uint32_t RESERVED6[2];          // 0x78, 0x7C - Reserved

    volatile uint32_t SSCGR;        // 0x80 - Spread spectrum clock generation register
    volatile uint32_t PLLI2SCFGR;   // 0x84 - PLLI2S configuration register
    volatile uint32_t PLLSAICFGR;   // 0x88 - PLLSAI configuration register
    volatile uint32_t DCKCFGR;      // 0x8C - Dedicated clocks configuration register
    volatile uint32_t CKGATENR;     // 0x90 - Clocks gated enable register
    volatile uint32_t DCKCFGR2;     // 0x94 - Dedicated clocks configuration register 2

} RCC_RegDef_t;

// BASE ADDRESSES OF CORTEX-M PERIPHERALS ============================================

#define SysTick_BASEADDR    0xE000E010UL


// SYSTICK PERIPHERAL REGISTER DEFINITION STRUCTURE ==================================

typedef struct
{
    volatile uint32_t CTRL;      // 0x00 - SysTick Control and Status Register
    volatile uint32_t LOAD;      // 0x04 - SysTick Reload Value Register
    volatile uint32_t VAL;       // 0x08 - SysTick Current Value Register
    volatile uint32_t CALIB;     // 0x0C - SysTick Calibration Value Register
} SysTick_RegDef_t;


// PERIPHERAL DEFINITION =============================================================

#define SysTick    ((SysTick_RegDef_t *)SysTick_BASEADDR)



// peripheral definitions

#define GPIOA   ((GPIO_RegDef_t*) GPIOA_BASEADDR)
#define GPIOB   ((GPIO_RegDef_t*) GPIOB_BASEADDR)
#define GPIOC   ((GPIO_RegDef_t*) GPIOC_BASEADDR)
#define GPIOD   ((GPIO_RegDef_t*) GPIOD_BASEADDR)
#define GPIOE   ((GPIO_RegDef_t*) GPIOE_BASEADDR)
#define GPIOF   ((GPIO_RegDef_t*) GPIOF_BASEADDR)
#define GPIOG   ((GPIO_RegDef_t*) GPIOG_BASEADDR)
#define GPIOH   ((GPIO_RegDef_t*) GPIOH_BASEADDR)


/* I2C peripheral pointers */

#define I2C1    ((I2C_RegDef_t *)I2C1_BASEADDR)
#define I2C2    ((I2C_RegDef_t *)I2C2_BASEADDR)
#define I2C3    ((I2C_RegDef_t *)I2C3_BASEADDR)


#define SPI1    ((SPI_RegDef_t *)SPI1_BASEADDR)
#define SPI2    ((SPI_RegDef_t *)SPI2_BASEADDR)
#define SPI3    ((SPI_RegDef_t *)SPI3_BASEADDR)
#define SPI4    ((SPI_RegDef_t *)SPI4_BASEADDR)


#define USART1    ((USART_RegDef_t *)USART1_BASEADDR)
#define USART2    ((USART_RegDef_t *)USART2_BASEADDR)
#define USART3    ((USART_RegDef_t *)USART3_BASEADDR)
#define UART4     ((USART_RegDef_t *)UART4_BASEADDR)
#define UART5     ((USART_RegDef_t *)UART5_BASEADDR)
#define USART6    ((USART_RegDef_t *)USART6_BASEADDR)
#define UART7     ((USART_RegDef_t *)UART7_BASEADDR)
#define UART8     ((USART_RegDef_t *)UART8_BASEADDR)


#define RCC   ((RCC_RegDef_t*) RCC_BASEADDR)



//===============================================================
// Clock Enable Macros for GPIOX Peripherals
//===============================================================

#define GPIOA_PCLK_EN()      (RCC->AHB1ENR |= (1 << 0))
#define GPIOB_PCLK_EN()      (RCC->AHB1ENR |= (1 << 1))
#define GPIOC_PCLK_EN()      (RCC->AHB1ENR |= (1 << 2))
#define GPIOD_PCLK_EN()      (RCC->AHB1ENR |= (1 << 3))
#define GPIOE_PCLK_EN()      (RCC->AHB1ENR |= (1 << 4))
#define GPIOF_PCLK_EN()      (RCC->AHB1ENR |= (1 << 5))
#define GPIOG_PCLK_EN()      (RCC->AHB1ENR |= (1 << 6))
#define GPIOH_PCLK_EN()      (RCC->AHB1ENR |= (1 << 7))


//===============================================================
// Clock Disable Macros for GPIOX Peripherals
//===============================================================

#define GPIOA_PCLK_DI()      (RCC->AHB1ENR &= ~(1 << 0))
#define GPIOB_PCLK_DI()      (RCC->AHB1ENR &= ~(1 << 1))
#define GPIOC_PCLK_DI()      (RCC->AHB1ENR &= ~(1 << 2))
#define GPIOD_PCLK_DI()      (RCC->AHB1ENR &= ~(1 << 3))
#define GPIOE_PCLK_DI()      (RCC->AHB1ENR &= ~(1 << 4))
#define GPIOF_PCLK_DI()      (RCC->AHB1ENR &= ~(1 << 5))
#define GPIOG_PCLK_DI()      (RCC->AHB1ENR &= ~(1 << 6))
#define GPIOH_PCLK_DI()      (RCC->AHB1ENR &= ~(1 << 7))


//===============================================================
// Clock Enable Macros for I2CX Peripherals
//===============================================================

#define I2C1_PCLK_EN()       (RCC->APB1ENR |= (1 << 21))
#define I2C2_PCLK_EN()       (RCC->APB1ENR |= (1 << 22))
#define I2C3_PCLK_EN()       (RCC->APB1ENR |= (1 << 23))


//===============================================================
// Clock Disable Macros for I2CX Peripherals
//===============================================================

#define I2C1_PCLK_DI()       (RCC->APB1ENR &= ~(1 << 21))
#define I2C2_PCLK_DI()       (RCC->APB1ENR &= ~(1 << 22))
#define I2C3_PCLK_DI()       (RCC->APB1ENR &= ~(1 << 23))


//===============================================================
// Clock Enable Macros for SPIX Peripherals
//===============================================================

// SPI1, SPI4 are on APB2
// SPI2, SPI3 are on APB1

#define SPI1_PCLK_EN()       (RCC->APB2ENR |= (1 << 12))
#define SPI4_PCLK_EN()       (RCC->APB2ENR |= (1 << 13))

#define SPI2_PCLK_EN()       (RCC->APB1ENR |= (1 << 14))
#define SPI3_PCLK_EN()       (RCC->APB1ENR |= (1 << 15))


//===============================================================
// Clock Disable Macros for SPIX Peripherals
//===============================================================

#define SPI1_PCLK_DI()       (RCC->APB2ENR &= ~(1 << 12))
#define SPI4_PCLK_DI()       (RCC->APB2ENR &= ~(1 << 13))

#define SPI2_PCLK_DI()       (RCC->APB1ENR &= ~(1 << 14))
#define SPI3_PCLK_DI()       (RCC->APB1ENR &= ~(1 << 15))


//===============================================================
// Clock Enable Macros for USARTX / UARTX Peripherals
//===============================================================

// USART1, USART6 are on APB2
// USART2, USART3, UART4, UART5 are on APB1

#define USART1_PCLK_EN()     (RCC->APB2ENR |= (1 << 4))
#define USART6_PCLK_EN()     (RCC->APB2ENR |= (1 << 5))

#define USART2_PCLK_EN()     (RCC->APB1ENR |= (1 << 17))
#define USART3_PCLK_EN()     (RCC->APB1ENR |= (1 << 18))
#define UART4_PCLK_EN()      (RCC->APB1ENR |= (1 << 19))
#define UART5_PCLK_EN()      (RCC->APB1ENR |= (1 << 20))


//===============================================================
// Clock Disable Macros for USARTX / UARTX Peripherals
//===============================================================

#define USART1_PCLK_DI()     (RCC->APB2ENR &= ~(1 << 4))
#define USART6_PCLK_DI()     (RCC->APB2ENR &= ~(1 << 5))

#define USART2_PCLK_DI()     (RCC->APB1ENR &= ~(1 << 17))
#define USART3_PCLK_DI()     (RCC->APB1ENR &= ~(1 << 18))
#define UART4_PCLK_DI()      (RCC->APB1ENR &= ~(1 << 19))
#define UART5_PCLK_DI()      (RCC->APB1ENR &= ~(1 << 20))


//===============================================================
// Clock Enable Macros for SYSCFG Peripheral
//===============================================================

#define SYSCFG_PCLK_EN()     (RCC->APB2ENR |= (1 << 14))


//===============================================================
// Clock Disable Macros for SYSCFG Peripheral
//===============================================================

#define SYSCFG_PCLK_DI()     (RCC->APB2ENR &= ~(1 << 14))

//===============================================================
//GPIO REGISTER RESET
//===============================================================

#define GPIOA_REG_RESET()      do { (RCC->AHB1RSTR |= (1 << 0)); (RCC->AHB1RSTR &= ~(1 << 0)); } while(0)

#define GPIOB_REG_RESET()      do { (RCC->AHB1RSTR |= (1 << 1)); (RCC->AHB1RSTR &= ~(1 << 1)); } while(0)

#define GPIOC_REG_RESET()      do { (RCC->AHB1RSTR |= (1 << 2)); (RCC->AHB1RSTR &= ~(1 << 2)); } while(0)

#define GPIOD_REG_RESET()      do { (RCC->AHB1RSTR |= (1 << 3)); (RCC->AHB1RSTR &= ~(1 << 3)); } while(0)

#define GPIOE_REG_RESET()      do { (RCC->AHB1RSTR |= (1 << 4)); (RCC->AHB1RSTR &= ~(1 << 4)); } while(0)

#define GPIOF_REG_RESET()      do { (RCC->AHB1RSTR |= (1 << 5)); (RCC->AHB1RSTR &= ~(1 << 5)); } while(0)

#define GPIOG_REG_RESET()      do { (RCC->AHB1RSTR |= (1 << 6)); (RCC->AHB1RSTR &= ~(1 << 6)); } while(0)

#define GPIOH_REG_RESET()      do { (RCC->AHB1RSTR |= (1 << 7)); (RCC->AHB1RSTR &= ~(1 << 7)); } while(0)



//SOME GENERIC MACROS============================================
#define ENABLE  		1
#define DISABLE 		0
#define SET				ENABLE
#define RESET			DISABLE
#define GPIO_PIN_SET	SET
#define GPIO_PIN_RESET	RESET


#include "stm32f446xx_gpio_driver.h"

#endif /* INC_STM32F446XX_H_ */


