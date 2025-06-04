#pragma once
#include <ch32v00x.h>
#include "debug.h"

void enableSWD()
{
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    AFIO->PCFR1 = AFIO_PCFR1_SWJ_CFG_RESET;
}

void disableSWD()
{
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    AFIO->PCFR1 = AFIO_PCFR1_SWJ_CFG_DISABLE;
}

void led(int on = -1)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
/*    if(on == -1)
        on = (GPIOA->OUTDR & 2)^2;

    if(on)
        GPIOA->BSHR = 2;
    else
        GPIOA->BCR = 2;*/
    if(on == -1)
        return;
    if(on)
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIOA->BSHR = 2;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }
    else
    {
        GPIOA->BCR = 2;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }
}

void blink(int ms = 100)
{
    led(1);
    Delay_Ms(ms >> 1);
    led(0);
    Delay_Ms(ms >> 1);
}

void initLED()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

volatile unsigned long time = 0;
void __attribute__((interrupt("WCH-Interrupt-fast"))) SysTick_Handler(void)
{
    time++;
    //led((time >> 17)&1);
    SysTick->SR=0;
    return;
}

void initTimerInterrupt()
{
    SysTick->SR=0;
    SysTick->CNT=0;
    SysTick->CMP=0x100;//0x100;
    SysTick->CTLR=1 | 2 | 4 | 8;
    NVIC_EnableIRQ(SysTicK_IRQn);
    SetVTFIRQ((u32)SysTick_Handler, SysTicK_IRQn, 0, ENABLE);
}

int getMcuIndex()
{
    return OB->Data0 & 0xff;
}

void setMcuIndex(uint8_t addr)
{
	uint8_t user = OB->USER & 0xff;

	if(	FLASH_EraseOptionBytes() != FLASH_COMPLETE)
	{
		blink();blink();blink();
		return;
	}
	if(FLASH_ProgramOptionByteData((uint32_t)&OB->USER, user) != FLASH_COMPLETE)
	{
		blink();blink();blink();
		return;
	}
	if(FLASH_ProgramOptionByteData((uint32_t)&OB->Data0, addr) != FLASH_COMPLETE)
	{
		blink();blink();blink();
		return;
	}
	blink(1000);
}

void unlockD7()
{
	if((OB->USER & 0b11000) == 0b11000) return; //reset already disabled
	blink();
	blink();
	blink();
    FLASH_Unlock();
    FLASH_EraseOptionBytes();
    FLASH_UserOptionByteConfig(OB_IWDG_SW, OB_STDBY_NoRST, OB_RST_NoEN, OB_PowerON_Start_Mode_USER);
    FLASH_Lock();
}