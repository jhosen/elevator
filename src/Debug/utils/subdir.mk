################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../utils/cJSON.c \
../utils/cb_buffer.c \
../utils/statemachine.c 

OBJS += \
./utils/cJSON.o \
./utils/cb_buffer.o \
./utils/statemachine.o 

C_DEPS += \
./utils/cJSON.d \
./utils/cb_buffer.d \
./utils/statemachine.d 


# Each subdirectory must supply rules for building sources it contributes
utils/%.o: ../utils/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


