################################################################################
# MRS Version: 1.9.2
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/UART/UART.c 

C_DEPS += \
./User/UART/UART.d 

OBJS += \
./User/UART/UART.o 


# Each subdirectory must supply rules for building sources it contributes
User/UART/%.o: ../User/UART/%.c
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\Debug" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\Core" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\User" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\Peripheral\inc" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\User\USBLIB\CONFIG" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\User\USBLIB\USB-Driver\inc" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\User\UART" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

