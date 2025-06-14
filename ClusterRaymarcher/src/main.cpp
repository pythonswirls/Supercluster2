#include <ch32v00x.h>
#include <stdio.h>
#include "timer.h"
#include "SystemClockfix.h"
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
	Vec3 pos;
	Vec3 dir;
	if(!readBusVec3(pos)) return false;
	if(!readBusVec3(dir)) return false;
	int depth = 2;
	led(1);
	pixelColor = renderPixel(pos, dir, depth);
	bus.outBuffer.write((uint8_t)BUS_RAYMARCHER_RENDER_PIXEL_RESULT);
	writeBusVec3(pixelColor);
	led(0);
	return true;
}

bool sendPing()
{
	bool ret = bus.outBuffer.write((uint8_t)BUS_PING);
	ledTimeout();
	return ret;
}

volatile uint16_t packetsLost = 0;
volatile uint16_t packetsConfirmed = 0;

void packetLost()
{
	packetsLost++;
	bus.inBuffer.clear();
	bus.outBuffer.write((uint8_t)BUS_CLIENT_ERROR);
}

void busLoop()
{
	uint8_t id = getMcuIndex();
	bus.id = id;
	while(1)
	{
		while(bus.state != Bus::STATE_IDLE || bus.inBuffer.size == 0)
		{
			processLedTimeOut();
		}
		uint8_t cmd = 0;
		bus.inBuffer.read(cmd);
		switch(cmd)
		{
			case BUS_CLIENT_SET_INDEX:
			{
				if(bus.inBuffer.size < 2)
				{
					packetLost();
					continue;
				}
				uint8_t newId;
				bus.inBuffer.read(newId);
				uint8_t checksum;
				bus.inBuffer.read(checksum);
				if(newId != static_cast<uint8_t>(~checksum)) break;
				id = newId;
				setMcuIndex(id);
				bus.id = id;
				break;
			}
			case BUS_LED:
				ledTimeout();
				break;

			case BUS_RAYMARCHER_INIT:
				break;
			case BUS_RAYMARCHER_RENDER_PIXEL:
				if(!renderPixel()) packetLost();
				break;
			case BUS_PING:
				sendPing();
				break;
			case BUS_CLIENT_RESET:
				bus.resetSignals(0);
				bus.outBuffer.clear();
				bus.inBuffer.clear();
				bus.state = Bus::STATE_IDLE;
				ledTimeout(500);
				break;
			default:
			//nope
				break;
		}
	}
}

int main(void)
{
	SetSysClockTo_48MHZ_HSIfix();
	SystemCoreClockUpdate();
    initDelayTimer();
    initLED();
    blink();
    delayMs(5000);
	unlockD7();
    //disableSWD(); 

    initPheripherals();
    blink();
    blink();
	loadScene();
	busLoop();
}
