################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../User/Src/Communication.cpp \
../User/Src/Emg.cpp \
../User/Src/Filter.cpp \
../User/Src/FlashManager.cpp \
../User/Src/LoadCell.cpp \
../User/Src/SystemAdapter.cpp 

OBJS += \
./User/Src/Communication.o \
./User/Src/Emg.o \
./User/Src/Filter.o \
./User/Src/FlashManager.o \
./User/Src/LoadCell.o \
./User/Src/SystemAdapter.o 

CPP_DEPS += \
./User/Src/Communication.d \
./User/Src/Emg.d \
./User/Src/Filter.d \
./User/Src/FlashManager.d \
./User/Src/LoadCell.d \
./User/Src/SystemAdapter.d 


# Each subdirectory must supply rules for building sources it contributes
User/Src/%.o User/Src/%.su User/Src/%.cyclo: ../User/Src/%.cpp User/Src/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-User-2f-Src

clean-User-2f-Src:
	-$(RM) ./User/Src/Communication.cyclo ./User/Src/Communication.d ./User/Src/Communication.o ./User/Src/Communication.su ./User/Src/Emg.cyclo ./User/Src/Emg.d ./User/Src/Emg.o ./User/Src/Emg.su ./User/Src/Filter.cyclo ./User/Src/Filter.d ./User/Src/Filter.o ./User/Src/Filter.su ./User/Src/FlashManager.cyclo ./User/Src/FlashManager.d ./User/Src/FlashManager.o ./User/Src/FlashManager.su ./User/Src/LoadCell.cyclo ./User/Src/LoadCell.d ./User/Src/LoadCell.o ./User/Src/LoadCell.su ./User/Src/SystemAdapter.cyclo ./User/Src/SystemAdapter.d ./User/Src/SystemAdapter.o ./User/Src/SystemAdapter.su

.PHONY: clean-User-2f-Src

