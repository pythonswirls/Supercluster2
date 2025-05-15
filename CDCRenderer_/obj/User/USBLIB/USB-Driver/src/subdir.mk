################################################################################
# MRS Version: 1.9.2
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/USBLIB/USB-Driver/src/usb_core.c \
../User/USBLIB/USB-Driver/src/usb_init.c \
../User/USBLIB/USB-Driver/src/usb_int.c \
../User/USBLIB/USB-Driver/src/usb_mem.c \
../User/USBLIB/USB-Driver/src/usb_regs.c \
../User/USBLIB/USB-Driver/src/usb_sil.c 

C_DEPS += \
./User/USBLIB/USB-Driver/src/usb_core.d \
./User/USBLIB/USB-Driver/src/usb_init.d \
./User/USBLIB/USB-Driver/src/usb_int.d \
./User/USBLIB/USB-Driver/src/usb_mem.d \
./User/USBLIB/USB-Driver/src/usb_regs.d \
./User/USBLIB/USB-Driver/src/usb_sil.d 

OBJS += \
./User/USBLIB/USB-Driver/src/usb_core.o \
./User/USBLIB/USB-Driver/src/usb_init.o \
./User/USBLIB/USB-Driver/src/usb_int.o \
./User/USBLIB/USB-Driver/src/usb_mem.o \
./User/USBLIB/USB-Driver/src/usb_regs.o \
./User/USBLIB/USB-Driver/src/usb_sil.o 


# Each subdirectory must supply rules for building sources it contributes
User/USBLIB/USB-Driver/src/%.o: ../User/USBLIB/USB-Driver/src/%.c
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\Debug" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\Core" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\User" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\Peripheral\inc" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\User\USBLIB\CONFIG" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\User\USBLIB\USB-Driver\inc" -I"C:\Users\luni\Videos\Supercluster2.0\pub2\Supercluster2\CDCRenderer\User\UART" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

