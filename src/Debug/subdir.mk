################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../communication.c \
../control.c \
../elevator.c \
../main.c \
../network.c \
../operator.c \
../order.c 

OBJS += \
./communication.o \
./control.o \
./elevator.o \
./main.o \
./network.o \
./operator.o \
./order.o 

C_DEPS += \
./communication.d \
./control.d \
./elevator.d \
./main.d \
./network.d \
./operator.d \
./order.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


