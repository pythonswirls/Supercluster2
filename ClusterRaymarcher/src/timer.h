#pragma once
#include <stdint.h>
#include <core_riscv.h>
#include "debug.h"

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
	SysTick->CMP=0x0;//0x100;
	SysTick->CTLR= 
		1 | // enable timer
		0 | // enable interrupt
		4 | // select clock source (HCLK) 0 = AHB/8, 1 = AHB
		0; // free running mode
	//Delay_Init();
}

inline uint32_t getTime()
{
	return SysTick->CNT;
}

inline uint32_t ms2ticks(uint32_t ms)
{
	return ms * 48000;
}

inline uint32_t us2ticks(uint32_t us)
{
	return us * 48;
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
	//Delay_Us(us);
	//return;
	delayTicks(us2ticks(us));
}

inline void delayMs(uint32_t ms)
{
	//Delay_Ms(ms);
	//return;
	delayTicks(ms2ticks(ms));
}