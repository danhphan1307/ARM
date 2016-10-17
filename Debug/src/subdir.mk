################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/DigitalIoPin.cpp \
../src/Motor.cpp \
../src/Servo.cpp \
../src/cr_cpp_config.cpp \
../src/cr_startup_lpc15xx.cpp \
../src/project.cpp 

C_SRCS += \
../src/crp.c \
../src/sysinit.c 

OBJS += \
./src/DigitalIoPin.o \
./src/Motor.o \
./src/Servo.o \
./src/cr_cpp_config.o \
./src/cr_startup_lpc15xx.o \
./src/crp.o \
./src/project.o \
./src/sysinit.o 

CPP_DEPS += \
./src/DigitalIoPin.d \
./src/Motor.d \
./src/Servo.d \
./src/cr_cpp_config.d \
./src/cr_startup_lpc15xx.d \
./src/project.d 

C_DEPS += \
./src/crp.d \
./src/sysinit.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -D__NEWLIB__ -DDEBUG -D__CODE_RED -DCORE_M3 -D__USE_LPCOPEN -DCPP_USE_HEAP -D__LPC15XX__ -I"/Users/danh/Documents/LPCXpresso_8.2.2/workspace/lpc_board_nxp_lpcxpresso_1549/inc" -I"/Users/danh/Documents/LPCXpresso_8.2.2/workspace/lpc_chip_15xx/inc" -I"/Users/danh/Documents/LPCXpresso_8.2.2/workspace/Project/src" -I"/Users/danh/Documents/LPCXpresso_8.2.2/workspace/Project/freertos/inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__NEWLIB__ -DDEBUG -D__CODE_RED -DCORE_M3 -D__USE_LPCOPEN -DCPP_USE_HEAP -D__LPC15XX__ -I"/Users/danh/Documents/LPCXpresso_8.2.2/workspace/lpc_board_nxp_lpcxpresso_1549/inc" -I"/Users/danh/Documents/LPCXpresso_8.2.2/workspace/lpc_chip_15xx/inc" -I"/Users/danh/Documents/LPCXpresso_8.2.2/workspace/rgb_led/src" -I"/Users/danh/Documents/LPCXpresso_8.2.2/workspace/rgb_led/freertos/inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


