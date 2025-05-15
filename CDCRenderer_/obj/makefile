################################################################################
# MRS Version: 1.9.2
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include User/USBLIB/USB-Driver/src/subdir.mk
-include User/USBLIB/CONFIG/subdir.mk
-include User/UART/subdir.mk
-include User/subdir.mk
-include Startup/subdir.mk
-include Peripheral/src/subdir.mk
-include Debug/subdir.mk
-include Core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
CDCRenderer.hex \

SECONDARY_LIST += \
CDCRenderer.lst \

SECONDARY_SIZE += \
CDCRenderer.siz \


# All Target
all: CDCRenderer.elf secondary-outputs

# Tool invocations
CDCRenderer.elf: $(OBJS) $(USER_OBJS)
	@	@	riscv-none-embed-g++ -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -T Link.ld -nostartfiles -Xlinker --gc-sections -L"../LD" -Wl,-Map,"CDCRenderer.map" --specs=nano.specs --specs=nosys.specs -o "CDCRenderer.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@	@
CDCRenderer.hex: CDCRenderer.elf
	@	riscv-none-embed-objcopy -O ihex "CDCRenderer.elf"  "CDCRenderer.hex"
	@	@
CDCRenderer.lst: CDCRenderer.elf
	@	riscv-none-embed-objdump --all-headers --demangle --disassemble -M xw "CDCRenderer.elf" > "CDCRenderer.lst"
	@	@
CDCRenderer.siz: CDCRenderer.elf
	@	riscv-none-embed-size --format=berkeley "CDCRenderer.elf"
	@	@
# Other Targets
clean:
	-$(RM) $(C_UPPER_DEPS)$(SECONDARY_LIST)$(SECONDARY_SIZE)$(S_DEPS)$(C_DEPS)$(CC_DEPS)$(ASM_UPPER_DEPS)$(C++_DEPS)$(OBJS)$(CXX_DEPS)$(SECONDARY_FLASH)$(ASM_DEPS)$(S_UPPER_DEPS)$(CPP_DEPS) CDCRenderer.elf
	-@
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
