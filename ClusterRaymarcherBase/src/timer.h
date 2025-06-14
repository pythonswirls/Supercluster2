#pragma once
#include <stdint.h>
#include <core_riscv.h>

/*
void __attribute__((interrupt("WCH-Interrupt-fast"))) SysTick_Handler(void)
{
    time++;
    SysTick->SR=0;
    return;
}*/

void initDelayTimer()
{
	SysTick->SR=0;
	SysTick->CNT=0;
	SysTick->CMP=0x80000000;//0x100;
	SysTick->CTLR= 
		1 | // enable timer
		0/*2*/ | // enable interrupt
		4 | // select clock source (HCLK) 0 = AHB/8, 1 = AHB
		0/*8*/; // free running mode
}

inline uint32_t getTime()
{
	return SysTick->CNT;
}

inline uint32_t ms2ticks(uint32_t ms)
{
	return ms * 144000;
}

inline uint32_t us2ticks(uint32_t us)
{
	return us * 144;
}

inline void delayTicks(uint32_t ticks)
{
	const uint32_t start = getTime();
	while (getTime() - start < ticks) 
	{
	};
}

inline void delayUs(uint32_t us)
{
	delayTicks(us2ticks(us));
}

inline void delayMs(uint32_t ms)
{
	delayTicks(ms2ticks(ms));
}