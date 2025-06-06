#include "debug.h"
#include "SystemClockfix.h"
#include <ch32v00x.h>
#include "raymarcher/raymarcher.h"
#include "utils.h"
#include "ClientBusCH32V003.h"

uint8_t *uid = (uint8_t*)0x1ffff7E8;

void initPheripherals()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	bus.init();
    initLED();

}


bool readBusVec3(Vec3 &vec)
{
	uint32_t d;
	if(!bus.inBuffer.read(d)) return false;
	vec.v[0] = (int32_t)d;
	if(!bus.inBuffer.read(d)) return false;
	vec.v[1] = (int32_t)d;
	if(!bus.inBuffer.read(d)) return false;
	vec.v[2] = (int32_t)d;
	return true;
}

bool writeBusVec3(Vec3 &vec)
{
	if(!bus.outBuffer.write((uint32_t)vec.v[0])) return false;
	if(!bus.outBuffer.write((uint32_t)vec.v[1])) return false;
	if(!bus.outBuffer.write((uint32_t)vec.v[2])) return false;
	return true;
}

Vec3 pixelColor;

bool renderPixel()
{
	if(bus.inBuffer.size < 24) return false;
	//blink(1000);	
	Vec3 pos;
	Vec3 dir;
	if(!readBusVec3(pos)) return false;
	if(!readBusVec3(dir)) return false;
	//blink(1000);	
	int depth = 2;
	pixelColor = renderPixel(pos, dir, depth);
	writeBusVec3(pixelColor);
//	if(!bus.outBuffer.write((uint32_t)dir.v[0])) return false;
	//if(!writeBusVec3(dir)) return false;
	blink(1000);
	return true;
}

bool sendPing()
{
	return bus.outBuffer.write((uint8_t)BUS_PING);
}

void busLoop()
{
	uint8_t id = getMcuIndex();
	bus.id = id;
	while(1)
	{
		while(bus.state != Bus::STATE_IDLE || bus.inBuffer.size == 0)
		{
			Delay_Us(10);
		}
		uint8_t cmd = 0;
		bus.inBuffer.read(cmd);
		switch(cmd)
		{
			case BUS_SET_INDEX:
			{
				if(!bus.inBuffer.read(id)) break;
				setMcuIndex(id);
				bus.id = id;
				break;
			}
			case BUS_LED:
				blink();
				break;

			case BUS_RAYMARCHER_INIT:
				break;
			case BUS_RAYMARCHER_RENDER_PIXEL:
				renderPixel();
				break;
			case BUS_PING:
				sendPing();
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
    Delay_Ms(5000);
	unlockD7();
    disableSWD(); 

    initPheripherals();
    blink();
    blink();
	loadScene();
	busLoop();
}
