#pragma once
#include <ch32v00x.h>

static void SetSysClockTo_48MHZ_HSIfix(void)
{
    uint8_t tmp = 0;

    tmp = *( uint8_t * )CFG0_PLL_TRIM;

    if(tmp != 0xFF)
    {
        RCC_AdjustHSICalibrationValue((tmp & 0x1F));
    }

    /* Flash 0 wait state */
    FLASH->ACTLR &= (uint32_t)((uint32_t)~FLASH_ACTLR_LATENCY);
    FLASH->ACTLR |= (uint32_t)FLASH_ACTLR_LATENCY_1;

    /* HCLK = SYSCLK = APB1 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;

    /* PLL configuration: PLLCLK = HSI * 2 = 48 MHz */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC));
    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSI_Mul2);

    /* Enable PLL */
    RCC->CTLR |= RCC_PLLON;
    /* Wait till PLL is ready */
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }
    /* Select PLL as system clock source */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;    
    /* Wait till PLL is used as system clock source */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }

	AFIO->PCFR1 = 0; //reenable GPIOA
}