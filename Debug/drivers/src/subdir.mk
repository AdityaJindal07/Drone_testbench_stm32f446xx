################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../drivers/src/stm32f446xx_gpio_driver.c \
../drivers/src/stm32f446xx_i2c_driver.c \
../drivers/src/stm32f446xx_pwm_driver.c \
../drivers/src/stm32f446xx_spi_driver.c \
../drivers/src/stm32f446xx_uart_driver.c 

OBJS += \
./drivers/src/stm32f446xx_gpio_driver.o \
./drivers/src/stm32f446xx_i2c_driver.o \
./drivers/src/stm32f446xx_pwm_driver.o \
./drivers/src/stm32f446xx_spi_driver.o \
./drivers/src/stm32f446xx_uart_driver.o 

C_DEPS += \
./drivers/src/stm32f446xx_gpio_driver.d \
./drivers/src/stm32f446xx_i2c_driver.d \
./drivers/src/stm32f446xx_pwm_driver.d \
./drivers/src/stm32f446xx_spi_driver.d \
./drivers/src/stm32f446xx_uart_driver.d 


# Each subdirectory must supply rules for building sources it contributes
drivers/src/%.o drivers/src/%.su drivers/src/%.cyclo: ../drivers/src/%.c drivers/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F4 -DSTM32F446RETx -c -I../Inc -I"/home/aditya-jindal007/STM32CubeIDE/workspace_2.1.1/STM32F4xx_drivers/drivers/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-drivers-2f-src

clean-drivers-2f-src:
	-$(RM) ./drivers/src/stm32f446xx_gpio_driver.cyclo ./drivers/src/stm32f446xx_gpio_driver.d ./drivers/src/stm32f446xx_gpio_driver.o ./drivers/src/stm32f446xx_gpio_driver.su ./drivers/src/stm32f446xx_i2c_driver.cyclo ./drivers/src/stm32f446xx_i2c_driver.d ./drivers/src/stm32f446xx_i2c_driver.o ./drivers/src/stm32f446xx_i2c_driver.su ./drivers/src/stm32f446xx_pwm_driver.cyclo ./drivers/src/stm32f446xx_pwm_driver.d ./drivers/src/stm32f446xx_pwm_driver.o ./drivers/src/stm32f446xx_pwm_driver.su ./drivers/src/stm32f446xx_spi_driver.cyclo ./drivers/src/stm32f446xx_spi_driver.d ./drivers/src/stm32f446xx_spi_driver.o ./drivers/src/stm32f446xx_spi_driver.su ./drivers/src/stm32f446xx_uart_driver.cyclo ./drivers/src/stm32f446xx_uart_driver.d ./drivers/src/stm32f446xx_uart_driver.o ./drivers/src/stm32f446xx_uart_driver.su

.PHONY: clean-drivers-2f-src

