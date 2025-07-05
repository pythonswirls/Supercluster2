#include <ch32v00x.h>
#include <stdio.h>
#include "timer.h"
#include "SystemClockfix.h"
#include "raymarcher/raymarcher.h"
#include "utils.h"
#include "ClientBusCH32V003.h"
#include "sha256.h"

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

bool readBusVec3Short(Vec3 &vec)
{
	uint16_t d;
	if(!bus.inBuffer.read(d)) return false;
	vec.v[0] = (int32_t)((int16_t)d) << 8;
	if(!bus.inBuffer.read(d)) return false;
	vec.v[1] = (int32_t)((int16_t)d) << 8;
	if(!bus.inBuffer.read(d)) return false;
	vec.v[2] = (int32_t)((int16_t)d) << 8;
	return true;
}

bool readBusVec3ShortNorm(Vec3 &vec)
{
	int16_t d;
	if(!bus.inBuffer.read(d)) return false;
	vec.v[0] = ((int32_t)d) << 1;
	if(!bus.inBuffer.read(d)) return false;
	vec.v[1] = ((int32_t)d) << 1;
	if(!bus.inBuffer.read(d)) return false;
	vec.v[2] = ((int32_t)d) << 1;
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
	if(bus.inBuffer.size < 6) return false;
	Vec3 dir;
	if(!readBusVec3ShortNorm(dir)) return false;
	int depth = 2;
	//led(1);
	pixelColor = renderPixel(scene->cameraPos, dir, depth).scale(0xff).clamp(0, 255);
	bus.busy = true;
	bus.outBuffer.write((uint8_t)BUS_RAYMARCHER_RENDER_PIXEL_RESULT);
	//writeBusVec3(pixelColor);
	bus.outBuffer.write((uint8_t)(pixelColor.v[0] & 0xff));
	bus.outBuffer.write((uint8_t)(pixelColor.v[1] & 0xff));
	bus.outBuffer.write((uint8_t)(pixelColor.v[2] & 0xff));
	bus.busy = false;
	//led(0);
	return true;
}

bool shaRunning = false;
uint8_t shabuffer[64];
uint16_t shaCount = 0;

void processSha256()
{
	if(!shaRunning) return;
	//led(1);
	SHA256::compute(shabuffer);
	//led(0);
	shaCount++;
	if(bus.outBuffer.size == 0)
	{
		//uint8_t count = shaCount > 255 ? 255 : shaCount;
		//bus.outBuffer.write((uint8_t)BUS_SHA256_COUNT);
		bus.busy = true;
		bus.outBuffer.write((uint8_t)BUS_SHA256_COUNT);
		bus.outBuffer.write(shaCount);
		bus.busy = false;
		shaCount = 0;
	}
}

bool shaStart()
{
	for(int i = 0; i < 64; i++)
		shabuffer[i] = 0;
	shabuffer[0] = getMcuIndex();
	shaCount = 0;
	shaRunning = true;
	return true;
}

bool shaStop()
{
	if(!shaRunning) return false;
	bus.outBuffer.clear();
	shaRunning = false;
	return true;
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
	while(1)
	{
		bus.inBuffer.clear();
		while(bus.state != Bus::STATE_IDLE || bus.inBuffer.size == 0)
		{
			bus.processTimeout();
			processLedTimeOut();
			processSha256();
		}
		uint8_t cmd = 0;
		bus.inBuffer.read(cmd);
		switch(cmd)
		{
			case BUS_CLIENT_SET_INDEX:
			{
				uint8_t newId = 0;
				bus.inBuffer.read(newId);
				uint8_t checksum = 0;
				bus.inBuffer.read(checksum);
				//if(bus.id != 255) break;
				if(newId != static_cast<uint8_t>(~checksum)) break;
				setMcuIndex(newId);
				bus.id = newId;
				break;
			}
			case BUS_LED:
				ledTimeout();
				break;

			case BUS_RAYMARCHER_INIT:
				break;
			case BUS_RAYMARCHER_CAM_POS:
			{
				Vec3 camPos;
				readBusVec3Short(camPos);
				//scene->cameraPos = camPos;
				break;
			}
			case BUS_RAYMARCHER_RENDER_PIXEL:
				if(!renderPixel()) packetLost();
				break;
			case BUS_PING:
			{
				size_t n = bus.inBuffer.size;
				bus.outBuffer.write((uint8_t)BUS_PING);
				for(size_t i = 0; i < n; i++)
				{
					uint8_t data = 0;
					bus.inBuffer.read(data);
					bus.outBuffer.write(data);
				}
				break;
			}
			case BUS_CLIENT_TIMINGS:
			{
				uint32_t t;
				bus.inBuffer.read(t);
				bus.timeoutSignalChange = us2ticks(t);
				bus.inBuffer.read(t);
				bus.timeoutResponse = us2ticks(t);
				break;
			}
			case BUS_CLIENT_RESET:
				shaStop();
				bus.resetSignals(0);
				bus.outBuffer.clear();
				bus.inBuffer.clear();
				bus.state = Bus::STATE_IDLE;
				ledTimeout(50);
				break;

			case BUS_SHA256_START:
				if(!shaStart()) packetLost();
				break;
			case BUS_SHA256_END:
				if(!shaStart()) packetLost();
				break;
			default:
				bus.inBuffer.clear();
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
    //blink();
    //delayMs(5000);
	unlockD7();
    //disableSWD(); 

    initPheripherals();
	uint8_t id = getMcuIndex();
	bus.id = id;
	delayMs(id * 20);
	blink(20);
	loadScene();
	busLoop();
}
