################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/balance.c \
../Src/esc.c \
../Src/main.c \
../Src/mpu_task.c \
../Src/pwm_task.c \
../Src/syscalls.c \
../Src/sysmem.c 

OBJS += \
./Src/balance.o \
./Src/esc.o \
./Src/main.o \
./Src/mpu_task.o \
./Src/pwm_task.o \
./Src/syscalls.o \
./Src/sysmem.o 

C_DEPS += \
./Src/balance.d \
./Src/esc.d \
./Src/main.d \
./Src/mpu_task.d \
./Src/pwm_task.d \
./Src/syscalls.d \
./Src/sysmem.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F4 -DSTM32F446RETx -c -I../Inc -I"/home/aditya-jindal007/STM32CubeIDE/workspace_2.1.1/STM32F4xx_drivers/drivers/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/balance.cyclo ./Src/balance.d ./Src/balance.o ./Src/balance.su ./Src/esc.cyclo ./Src/esc.d ./Src/esc.o ./Src/esc.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/mpu_task.cyclo ./Src/mpu_task.d ./Src/mpu_task.o ./Src/mpu_task.su ./Src/pwm_task.cyclo ./Src/pwm_task.d ./Src/pwm_task.o ./Src/pwm_task.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su

.PHONY: clean-Src

