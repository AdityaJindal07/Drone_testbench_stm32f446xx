/*
 * 002LedBtn.c
 *
 *  Created on: Oct 1, 2026
 *      Author: aditya-jindal007
 */


#include "stm32f446xx.h"

#define btn_pressed 0

void delay(void){
	for(uint32_t i=0;i<500000/2;i++);
}

int main(void){
	GPIO_Handle_t GpioLed, LedBtn;
	GpioLed.pGPIOx = GPIOA;
	GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
	GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PeriClkControl(GPIOA,ENABLE);
	GPIO_Init(&GpioLed);

	LedBtn.pGPIOx = GPIOC;
	LedBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	LedBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	LedBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	LedBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PeriClkControl(GPIOC,ENABLE);
	GPIO_Init(&LedBtn);


	while(1){
		if(GPIO_ReadFromInputPin(GPIOC,GPIO_PIN_NO_13) == btn_pressed){
			delay();
			GPIO_ToggleOutputPin(GPIOA,GPIO_PIN_NO_5);

		}
	}


}
