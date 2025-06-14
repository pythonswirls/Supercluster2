#pragma once
#include <ch32v00x.h>
#include "timer.h"

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
    if(on == 1)
		GPIOA->CFGLR = (GPIOA->CFGLR & 0xf00) | 0x80;
    else if(on == 255)
		GPIOA->CFGLR = (GPIOA->CFGLR & 0xf00) | 0x30;
	else
		GPIOA->CFGLR = (GPIOA->CFGLR & 0xf00) | 0x40;
}

void blink(int ms = 100)
{
    led(255);
    delayMs(ms >> 1);
    led(0);
    delayMs(ms >> 1);
}

volatile uint32_t ledPower = 1;
volatile uint32_t ledTimeoutTicks = 0;
void ledTimeout(uint32_t time = 100)
{
	resetTimer();
	led(ledPower);
	ledTimeoutTicks = ms2ticks(time);
}

void processLedTimeOut()
{
	if(ledTimeoutTicks)
		if(getTime() >= ledTimeoutTicks)
			led(0);
}

void initLED()
{
	/*NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	GPIO_InitTypeDef GPIO_InitStructure = {0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIOA->BSHR = 2;
}

int getMcuIndex()
{
    return OB->Data0 & 0xff;
}

void FlashOptionData(uint8_t data0, uint8_t data1) //taken from ch32fun, thx
{
	constexpr uint32_t FLASH_KEY1 = 0x45670123;
	constexpr uint32_t FLASH_KEY2 = 0xCDEF89AB;
	constexpr uint32_t CR_OPTPG_Set = ((uint32_t)0x00000010);
	constexpr uint32_t CR_OPTPG_Reset = ((uint32_t)0xFFFFFFEF);
	constexpr uint32_t CR_OPTER_Set = ((uint32_t)0x00000020);
	constexpr uint32_t CR_OPTER_Reset = ((uint32_t)0xFFFFFFDF);
	constexpr uint32_t CR_STRT_Set = ((uint32_t)0x00000040);
	constexpr uint32_t CR_LOCK_Set = ((uint32_t)0x00000080);

	volatile uint16_t hold[6];
	uint32_t *hold32p=(uint32_t *)hold;
	uint32_t *ob32p=(uint32_t *)OB_BASE;
	hold32p[0]=ob32p[0];            // Copy RDPR and USER
	hold32p[1]=data0+(data1<<16);   // Copy in the two Data values to be written
	hold32p[2]=ob32p[2];            // Copy WRPR0 and WEPR1

	// Unlock both the general Flash and the User-selected words
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;
	FLASH->OBKEYR = FLASH_KEY1;
	FLASH->OBKEYR = FLASH_KEY2;

	FLASH->CTLR |= CR_OPTER_Set;            // OBER RW Perform user-selected word erasure
	FLASH->CTLR |= CR_STRT_Set;             // STRT RW1 Start. Set 1 to start an erase action,hw automatically clears to 0
	while (FLASH->STATR & FLASH_BUSY);      // Wait for flash operation to be done
	FLASH->CTLR &= CR_OPTER_Reset;          // Disable erasure mode

	// Write the held values back one-by-one
	FLASH->CTLR |= CR_OPTPG_Set;            // OBG  RW Perform user-selected word programming
	uint16_t *ob16p=(uint16_t *)OB_BASE;
	for (uint32_t i = 0; i < sizeof(hold)/sizeof(hold[0]); i++) 
	{
		ob16p[i]=hold[i];
		while (FLASH->STATR & FLASH_BUSY);  // Wait for flash operation to be done
	}
	FLASH->CTLR &= CR_OPTPG_Reset;          // Disable programming mode

	FLASH->CTLR|=CR_LOCK_Set;               // Lock flash memories again

	return;
}

void setMcuIndex(uint8_t addr)
{
	FlashOptionData(addr, 0); //set Data0 to addr, Data1 is not used
	/*
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
	}*/
	ledTimeout(1000);
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