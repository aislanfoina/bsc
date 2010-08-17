################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ClusterGenerator.c \
../src/ProfileAnalyses.c \
../src/ProfileGenerator.c \
../src/SystemMaintence.c \
../src/main.c 

OBJS += \
./src/ClusterGenerator.o \
./src/ProfileAnalyses.o \
./src/ProfileGenerator.o \
./src/SystemMaintence.o \
./src/main.o 

C_DEPS += \
./src/ClusterGenerator.d \
./src/ProfileAnalyses.d \
./src/ProfileGenerator.d \
./src/SystemMaintence.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include/mysql -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


