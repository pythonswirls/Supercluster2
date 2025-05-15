################################################################################
# MRS Version: 1.9.2
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../User/main.cpp 

C_SRCS += \
../User/ch32v20x_it.c \
../User/system_ch32v20x.c 

C_DEPS += \
./User/ch32v20x_it.d \
./User/system_ch32v20x.d 

OBJS += \
./User/ch32v20x_it.o \
./User/main.o \
./User/system_ch32v20x.o 

CPP_DEPS += \
./User/main.d 


# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\Debug" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\Core" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\User" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\Peripheral\inc" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\User\USBLIB\CONFIG" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\User\USBLIB\USB-Driver\inc" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\User\UART" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
User/%.o: ../User/%.cpp
	@	@	riscv-none-embed-g++ -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\Debug" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\Core" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\User" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\Peripheral\inc" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\User\USBLIB\CONFIG" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\User\USBLIB\USB-Driver\inc" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\User\UART" -std=gnu++11 -fabi-version=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

