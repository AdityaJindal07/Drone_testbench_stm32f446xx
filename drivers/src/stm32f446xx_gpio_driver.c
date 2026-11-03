/*
 * stm32f446xx_gpio_driver.c
 *
 *  Created on: Sep 26, 2026
 *      Author: aditya-jindal007
 */

#include "stm32f446xx_gpio_driver.h"

/* GPIO Peripheral Clock Control */

void GPIO_PeriClkControl(GPIO_RegDef_t *pGPIOx, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        if(pGPIOx == GPIOA)
        {
            GPIOA_PCLK_EN();
        }
        else if(pGPIOx == GPIOB)
        {
            GPIOB_PCLK_EN();
        }
        else if(pGPIOx == GPIOC)
        {
            GPIOC_PCLK_EN();
        }
        else if(pGPIOx == GPIOD)
        {
            GPIOD_PCLK_EN();
        }
        else if(pGPIOx == GPIOE)
        {
            GPIOE_PCLK_EN();
        }
        else if(pGPIOx == GPIOF)
        {
            GPIOF_PCLK_EN();
        }
        else if(pGPIOx == GPIOG)
        {
            GPIOG_PCLK_EN();
        }
        else if(pGPIOx == GPIOH)
        {
            GPIOH_PCLK_EN();
        }
    }
    else
    {
        if(pGPIOx == GPIOA)
        {
            GPIOA_PCLK_DI();
        }
        else if(pGPIOx == GPIOB)
        {
            GPIOB_PCLK_DI();
        }
        else if(pGPIOx == GPIOC)
        {
            GPIOC_PCLK_DI();
        }
        else if(pGPIOx == GPIOD)
        {
            GPIOD_PCLK_DI();
        }
        else if(pGPIOx == GPIOE)
        {
            GPIOE_PCLK_DI();
        }
        else if(pGPIOx == GPIOF)
        {
            GPIOF_PCLK_DI();
        }
        else if(pGPIOx == GPIOG)
        {
            GPIOG_PCLK_DI();
        }
        else if(pGPIOx == GPIOH)
        {
            GPIOH_PCLK_DI();
        }
    }
}


/* GPIO Initialization / De-initialization */

//void GPIO_Init(GPIO_Handle_t *pGPIOHandle)
//{
//    uint32_t temp = 0;
//
//    // CONFIGURE: MODE,SPEED,PUPD SETTING,OPTYPE,ALTERNATE FUNCTIONALITY
//
//    /* 1. configure the mode */
//    if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode <= GPIO_MODE_ANALOG)
//    {
//        temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode
//                << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
//        pGPIOHandle->pGPIOx->MODER &= ~(0x3 << 2U * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
//        pGPIOHandle->pGPIOx->MODER |= temp;
//    }
//    else
//    {
//        // interrupt mode configuration
//    }
//
//    temp = 0;
//
//    /* 2. configure the speed */
//    temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinSpeed
//            << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
//    pGPIOHandle->pGPIOx->OSPEEDR &= ~(0x3 << 2U * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
//    pGPIOHandle->pGPIOx->OSPEEDR |= temp;
//
//    temp = 0;
//
//    /* 3. configure the pupd settings */
//    temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinPuPdControl
//            << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
//    pGPIOHandle->pGPIOx->PUPDR &= ~(0x3 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
//    pGPIOHandle->pGPIOx->PUPDR |= temp;
//
//    temp = 0;
//
//    /* 4. configure the optype */
//    temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinOPType
//            << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
//    pGPIOHandle->pGPIOx->OTYPER &= ~(0x1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
//    pGPIOHandle->pGPIOx->OTYPER |= temp;
//
//    temp = 0;
//
//    /* 5. configure the alt functionality */
//    if(pGPIOHandle -> GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_ALTFN){
//    	uint32_t temp1,temp2;
//
//    	temp1 = pGPIOHandle -> GPIO_PinConfig.GPIO_PinNumber / 8;
//    	temp2 = pGPIOHandle -> GPIO_PinConfig.GPIO_PinNumber % 8;
//    	pGPIOHandle->pGPIOx->AFR[temp1] &= ~(0xF  << (4 * temp2));
//    	pGPIOHandle->pGPIOx->AFR[temp1] |= pGPIOHandle -> GPIO_PinConfig.GPIO_PinAltFunMode << 4 * temp2;
//    }
//
//}

void GPIO_Init(GPIO_Handle_t *pGPIOHandle)
{
    uint32_t temp = 0;

    /* 1. Configure Pin Mode */
    if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode <= GPIO_MODE_ANALOG)
    {
        temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
        pGPIOHandle->pGPIOx->MODER &= ~(0x3U << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
        pGPIOHandle->pGPIOx->MODER |= temp;
    }

    /* 2. Configure Speed */
    temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinSpeed << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
    pGPIOHandle->pGPIOx->OSPEEDR &= ~(0x3U << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
    pGPIOHandle->pGPIOx->OSPEEDR |= temp;

    /* 3. Configure Pull-up/Pull-down (FIXED SHIFT) */
    temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinPuPdControl << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
    pGPIOHandle->pGPIOx->PUPDR &= ~(0x3U << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber)); /* Fixed: shifted by 2 * PinNumber */
    pGPIOHandle->pGPIOx->PUPDR |= temp;

    /* 4. Configure Output Type */
    temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinOPType << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
    pGPIOHandle->pGPIOx->OTYPER &= ~(0x1U << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
    pGPIOHandle->pGPIOx->OTYPER |= temp;

    /* 5. Configure Alternate Functionality */
    if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_ALTFN)
    {
        uint32_t temp1, temp2;

        temp1 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber / 8;
        temp2 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber % 8;
        pGPIOHandle->pGPIOx->AFR[temp1] &= ~(0xFU << (4 * temp2));
        pGPIOHandle->pGPIOx->AFR[temp1] |= (pGPIOHandle->GPIO_PinConfig.GPIO_PinAltFunMode << (4 * temp2));
    }
}

void GPIO_DeInit(GPIO_RegDef_t *pGPIOx){

	    if(pGPIOx == GPIOA)
	    {
	        GPIOA_REG_RESET();
	    }
	    else if(pGPIOx == GPIOB)
	    {
	        GPIOB_REG_RESET();
	    }
	    else if(pGPIOx == GPIOC)
	    {
	        GPIOC_REG_RESET();
	    }
	    else if(pGPIOx == GPIOD)
	    {
	        GPIOD_REG_RESET();
	    }
	    else if(pGPIOx == GPIOE)
	    {
	        GPIOE_REG_RESET();
	    }
	    else if(pGPIOx == GPIOF)
	    {
	        GPIOF_REG_RESET();
	    }
	    else if(pGPIOx == GPIOG)
	    {
	        GPIOG_REG_RESET();
	    }
	    else if(pGPIOx == GPIOH)
	    {
	        GPIOH_REG_RESET();
	    }
	}


/* GPIO Read and Write*/
uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber){
	uint8_t value;
	value = (uint8_t)((pGPIOx->IDR >> PinNumber) & 0x00000001);
	return value;
}
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx){
	uint16_t value;
		value = (uint8_t)pGPIOx->IDR;
		return value;
}
void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber, uint8_t value){
	if(value == GPIO_PIN_SET){
		pGPIOx->ODR |= (1 << PinNumber);
	}
	else{
		pGPIOx->ODR &= ~(1 << PinNumber);
	}
}
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint8_t value){
	pGPIOx->ODR = value;
}
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber){
	pGPIOx-> ODR ^= (1 << PinNumber);
}

/* GPIO Interrupt Configuration / Handling */
void GPIO_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnorDi);
void GPIO_IRQHandling(uint8_t PinNumber);

