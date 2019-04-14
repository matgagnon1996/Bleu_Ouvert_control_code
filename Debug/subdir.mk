################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ADS1115.c \
../FSM_crusher.c \
../FSM_extruder.c \
../FSM_main.c \
../MAX6675.c \
../RPI_Control_main.c \
../SPIDevice.c \
../asservissementTemperature.c \
../clientPi.c \
../communication.c \
../crusher.c \
../gb_common.c \
../hx711.c \
../pololu.c \
../read_scale.c 

OBJS += \
./ADS1115.o \
./FSM_crusher.o \
./FSM_extruder.o \
./FSM_main.o \
./MAX6675.o \
./RPI_Control_main.o \
./SPIDevice.o \
./asservissementTemperature.o \
./clientPi.o \
./communication.o \
./crusher.o \
./gb_common.o \
./hx711.o \
./pololu.o \
./read_scale.o 

C_DEPS += \
./ADS1115.d \
./FSM_crusher.d \
./FSM_extruder.d \
./FSM_main.d \
./MAX6675.d \
./RPI_Control_main.d \
./SPIDevice.d \
./asservissementTemperature.d \
./clientPi.d \
./communication.d \
./crusher.d \
./gb_common.d \
./hx711.d \
./pololu.d \
./read_scale.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I/home/mathieu/Documents/toolchain/eclipse-include/include -I"/home/mathieu/eclipse-workspace/Pi_Common_code/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


