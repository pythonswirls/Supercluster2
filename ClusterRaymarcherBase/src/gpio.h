#pragma once
#include <stdint.h>

uint64_t *uid = (uint64_t*)0x1ffff7E8;

GPIO_TypeDef *gndBank[] =     {GPIOA, GPIOA, GPIOA, GPIOD, GPIOA, GPIOA, GPIOA, GPIOD,
                                GPIOA, GPIOA, GPIOA, GPIOD, GPIOA, GPIOD, GPIOD, GPIOA,};
const int gndPin[] =                {8, 4, 0, 3, 1, 5, 9, 4,
                                 2, 6, 10, 5, 3, 6, 2, 7,};

GPIO_TypeDef *busBank[] =     {GPIOB, GPIOB, GPIOB, GPIOB, GPIOB, GPIOB, GPIOB, GPIOB,
                                    GPIOB, GPIOB, GPIOB, GPIOB, GPIOB, GPIOB, GPIOB, GPIOB,};
const int busPin[] =                {0, 1, 2, 3, 4, 5, 6, 7,
                                 8, 9, 10, 11, 12, 13, 14, 15,};

GPIO_TypeDef *mcuBank[] =     { GPIOC, GPIOC, GPIOC, GPIOC, GPIOC, GPIOC, GPIOC, GPIOC,
                                GPIOC, GPIOC, GPIOC, GPIOC, GPIOC, GPIOC, GPIOC, GPIOC,};
const int mcuPin[] =                {0, 1, 2, 3, 4, 5, 6, 7,
                                 8, 9, 10, 11, 12, 13, 14, 15,};


const int buttonPin = 15;
GPIO_TypeDef *buttonBank = GPIOA;

void initGpio()
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    //not working
    for(int i = 0; i < 16; i++)
    {
        GPIO_InitStructure.GPIO_Pin = 1 << gndPin[i];
        GPIO_Init(gndBank[i], &GPIO_InitStructure);
        GPIO_WriteBit(gndBank[i], 1 << gndPin[i], Bit_RESET);
    }

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = 1 << buttonPin;
    GPIO_Init(buttonBank, &GPIO_InitStructure);

	//MCU Pins
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    for(int i = 0; i < 16; i++)
    {
        GPIO_InitStructure.GPIO_Pin = 1 << mcuPin[i];
        GPIO_Init(mcuBank[i], &GPIO_InitStructure);
        GPIO_WriteBit(mcuBank[i], 1 << mcuPin[i], Bit_RESET);
    }

	//BUS pins
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    for(int i = 0; i < 16; i++)
    {
        GPIO_InitStructure.GPIO_Pin = 1 << busPin[i];
        GPIO_Init(busBank[i], &GPIO_InitStructure);
        GPIO_WriteBit(busBank[i], 1 << busPin[i], Bit_RESET);
    }

    int cl = 0;
    int ch = 0;
    ////input mode (mode with pull up and pull down)
	//output mode open drain
	GPIOC->BSHR = 0xffff;
    for(int i = 0; i < 16; i++)
    {
        //cl |= 0b1000 << (i * 4);
        //ch |= 0b1000 << (i * 4);
        cl |= 0b0100 << (i * 4);
        ch |= 0b0100 << (i * 4);
    }
    GPIOC->CFGLR = cl;
    GPIOC->CFGHR = ch;

    
	//Delay_Ms(3500);
    /*
    for(int i = 0; i < 16; i++)
    {
        setBusData(i, BUS_SET_INDEX);
        Delay_Ms(10);
        setMcuPin(i, 1);
        Delay_Ms(200);
        setMcuPin(i, 0);
    }*/
}

/** state 0/1 **/
void setMcuPin(int mcu, int state)
{
    if(state)
    {
        GPIOC->BSHR = 1 << mcu;
    }
    else
    {
        GPIOC->BCR = 1 << mcu;
    }
}

void button()
{
//        if(!((buttonBank->INDR >> buttonPin) & 1))
	//            while(!((buttonBank->INDR >> buttonPin) & 1));	
}