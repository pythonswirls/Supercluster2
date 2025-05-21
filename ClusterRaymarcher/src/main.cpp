#include "debug.h"
#include "SystemClockfix.h"
#include <ch32v00x.h>
#include "raymarcher/raymarcher.h"
#include "bus.h"

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
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        GPIOA->BSHR = 2;
    }
    else
    {
        GPIOA->BCR = 2;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }
}

uint8_t *uid = (uint8_t*)0x1ffff7E8;

volatile bool flashModeActive = false;

bool nrst()
{
    return (GPIOD->INDR >> 7) & 1;
}

volatile unsigned long time = 0;
void __attribute__((interrupt("WCH-Interrupt-fast"))) SysTick_Handler(void)
{
    time++;
    //led((time >> 17)&1);
    SysTick->SR=0;
    return;
}

int getMcuIndex()
{
    return OB->Data1;//(FLASH->OBR >> 10) & 0xff;//*(uint8_t*)(OB_BASE + 4);
}

void writeUserByte0(uint8_t b)
{
    FLASH_Unlock();
    FLASH_ProgramOptionByteData(OB_BASE + 4, b);
    FLASH_Lock();
}

void writeUserByte1(uint8_t b)
{
    FLASH_Unlock();
    FLASH_ProgramOptionByteData(OB_BASE + 6, b);
    FLASH_Lock();
}

uint8_t readUserByte0()
{
    return OB->Data0;
//    return *(uint8_t*)(OB_BASE + 4);
}

uint8_t readUserByte1()
{
    return OB->Data1;
//    return *(uint8_t*)(OB_BASE + 6);
}

void unlockD7()
{
    //if((FLASH_GetUserOptionByte() & OB_RST_NoEN) == OB_RST_NoEN) return; //already configured
    FLASH_Unlock();
    FLASH_EraseOptionBytes();
    FLASH_UserOptionByteConfig(OB_IWDG_SW, OB_STOP_NoRST, OB_STDBY_NoRST, OB_RST_NoEN);
    FLASH_Lock();
}

volatile bool debugActive = true;

void initLED()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void initPheripherals()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    initLED();
    initBus();
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

void FlashOptionData(uint8_t data0, uint8_t data1) //taken from ch32fun
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

bool writeID()
{
    int addr = readBusA() & 0xf;
    int inst = readBusB();
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) && (inst == BUS_SET_INDEX))
    {
        led(1);
        Delay_Ms(200);
        led(0);
        Delay_Ms(10);
        FlashOptionData(addr, 0);
        while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1));
        return true;
    }
    return false;
}

void setMcuIndex(uint8_t addr)
{
	led(1);
	Delay_Ms(200);
	led(0);
	Delay_Ms(10);
	FlashOptionData(addr, 0);
	//doesn't hurt, even hurts flash less
	while(getIo0());
}

void blink(int ms = 100)
{
    led(1);
    Delay_Ms(ms >> 1);
    led(0);
    Delay_Ms(ms >> 1);
}

bool readBusVec3(uint8_t id, Vec3 &vec)
{
	int32_t i;
	if(!readBusInt32(id, i)) return false;
	vec.v[0] = i;
	if(!readBusInt32(id, i)) return false;
	vec.v[1] = i;
	if(!readBusInt32(id, i)) return false;
	vec.v[2] = i;
	return true;
}

bool writeBusVec3(uint8_t id, Vec3 &vec)
{
	if(!writeBusInt32(id, vec.v[0])) return false;
	if(!writeBusInt32(id, vec.v[1])) return false;
	if(!writeBusInt32(id, vec.v[2])) return false;
	return true;
}

//globals
Vec3 pixelColor;

bool renderPixel(uint8_t id)
{
	Vec3 pos;
	Vec3 dir;
	if(readBusVec3(id, pos)) return false;
	if(readBusVec3(id, dir)) return false;
	int depth = 2;
	pixelColor = renderPixel(pos, dir, depth);
	return true;
}

bool sendRenderPixelResult(uint8_t id)
{
	if(!writeBusVec3(id, pixelColor)) return false;
	return true;
}

void busLoop()
{
	int id = getMcuIndex();
	while(1)
	{
		while(getIo1())
		{
			//wait for next command;
		}
		while(!getIo1())
		{
			//idle
		}
		//read command and address
		int addr = readBusA();
		int cmd = readBusB();
		//check if talking to us OR if setting the Mcu index
		if(cmd != BUS_SET_INDEX && addr != id && addr != 0xff) 
			continue;
		switch(cmd)
		{
			case BUS_SET_INDEX:
				if(getIo0()) setMcuIndex(addr);
				id = addr;
				break;
			case BUS_BLINK:
				blink();
				break;

			case BUS_RAYMARCHER_INIT:
				break;
			case BUS_RAYMARCHER_RENDER_PIXEL:
				renderPixel(id);
				break;
			case BUS_RAYMARCHER_RENDER_PIXEL_RESULT:
				sendRenderPixelResult(id);
				break;
			default:
			//nope
				break;
		}
	}
}

int main(void)
{
	SetSysClockTo_48MHz_HSEfix();
	SystemCoreClockUpdate();
    Delay_Init();
    initLED();
    blink();
    Delay_Ms(3000);
//  if(readUserByte1())
//    unlockD7();
    disableSWD();

    initPheripherals();
    blink();
/*    if(readUserByte1() == 255)
    {
        blink();
        blink();
        programmingPhase();
    }
    else
    {
        blink();
    }*/
	loadScene();
	busLoop();
}
