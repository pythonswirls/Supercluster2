#pragma once
#include <stdint.h>
#include <core_riscv.h>

const int ticks = 0x80;
volatile uint32_t time = 0;

void __attribute__((interrupt("WCH-Interrupt-fast"))) SysTick_Handler(void)
{
    time++;
    SysTick->SR=0;
    return;
}
