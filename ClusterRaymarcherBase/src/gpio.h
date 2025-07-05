#pragma once
#include <ch32v20x.h>
#include <stdint.h>

uint64_t *uid = (uint64_t*)0x1ffff7E8;


const int buttonPin = 15;
GPIO_TypeDef *buttonBank = GPIOA;

void initGpio()
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = 1 << buttonPin;
    GPIO_Init(buttonBank, &GPIO_InitStructure);
}
