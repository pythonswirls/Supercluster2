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

/*
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
}*/

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
			Delay_Us(100);
		}
		uint8_t cmd = 0;
		bus.inBuffer.read(cmd);
		switch(cmd)
		{
			case BUS_SET_INDEX:
			{
				bus.inBuffer.read(id);
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
				//renderPixel(id);
				break;
			case BUS_RAYMARCHER_RENDER_PIXEL_RESULT:
				//sendRenderPixelResult(id);
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
