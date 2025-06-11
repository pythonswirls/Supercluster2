constexpr bool BOARD_TYPE_PROGRAMMER = true;
constexpr bool BOARD_TYPE_BASE = false;

#include "usb/usb_serial.h"
#include "systemFix.h"
#include "raymarcher/raymarcher.h"
#include <debug.h>
#include "gpio.h"
#include "HostBusCH32V208.h"
#include "mcu.h"


USBSerial Serial;

class Serializer
{
public:
	Serializer()
	{
//		Serial.begin(115200);
	}

	uint8_t getUint8()
	{
		return Serial.read();
	}

	int32_t readInt32()
	{
		uint8_t buffer[4];
		buffer[0] = Serial.read();
		buffer[1] = Serial.read();
		buffer[2] = Serial.read();
		buffer[3] = Serial.read();
		return *(int32_t*)buffer;
	}

	void writeUint8(uint8_t value)
	{
		Serial.write(value);
	}

	void writeInt32(int32_t value)
	{
		uint8_t buffer[4];
		buffer[0] = value & 0xFF;
		buffer[1] = (value >> 8) & 0xFF;
		buffer[2] = (value >> 16) & 0xFF;
		buffer[3] = (value >> 24) & 0xFF;
		Serial.write(buffer, sizeof(buffer));
	}

	void flush()
	{
		Serial.flush();
	}
};

Serializer ser;

/// old programmer //////////////////////////////////////

void sendBlink(int addr)
{
	const uint8_t data[] = {BUS_LED};
	
	if(addr == 255)
		bus.sendBroadcast(0xffff, data, 1);
	else
	{
		HostBus::ErrorCode code = bus.sendPacket(1 << (addr & 15), addr, data, 1);
	//	Delay_Ms(100);
		ser.writeUint8(1);
		ser.writeUint8((int8_t)code);
		ser.flush();
	}
}

void pollResults()
{				
	for(int i = 0; i < MAX_MCUS; i++)
	{
		if(mcuStates[i] == MCU_RAYMARCHING)
		{
			int size = 0;
			if(bus.receivePacket(1 << (i & 0xf), i, 
				(uint8_t*)&(mcuRenderResult[i][mcuRenderResultSize[i]]), 
				size, 
				13 - mcuRenderResultSize[i]) != HostBus::ERROR_SUCCESS)
				continue;
			mcuRenderResultSize[i] += size;	
			
			if(mcuRenderResultSize[i] >= 1)
			{
				switch(mcuRenderResult[i][0])
				{
					case BUS_PACKET_LOST:
						mcuStates[i] = MCU_IDLE;
						bus.sendReset(1 << (i & 0xf), i);						
						ser.writeUint8(2);
						ser.writeUint8(BUS_PACKET_LOST);
						ser.writeUint8(i);
						ser.flush();
						continue;
					case BUS_RAYMARCHER_RENDER_PIXEL_RESULT:
						if(mcuRenderResultSize[i] >= 13)
						{
							mcuStates[i] = MCU_IDLE;
							ser.writeUint8(14);
							ser.writeUint8(BUS_RAYMARCHER_RENDER_PIXEL_RESULT);
							ser.writeUint8(i);
							for(int j = 0; j < 12; j++)
								ser.writeUint8(mcuRenderResult[i][j + 1]);
							ser.flush();
							break;
						}
					default:
						break;
				}
			}
		}
	}
}

int main(void)
{
	SetSysClockTo144_HSIfix();
	SystemCoreClockUpdate();
	Delay_Init();
	Delay_Ms(100);
	Serial.begin(115200);
	initGpio();
	bus.init();
	Delay_Ms(6000);
	initMCUs();

    while(1)
    {
		switch(ser.getUint8())
		{
			case BUS_SET_INDEX:
			{
				uint8_t baseIndex = ser.getUint8();
				for(uint8_t i = 0; i < 16; i++)
				{
					uint8_t data[3];
					data[0] = BUS_SET_INDEX;
					data[1] = (uint8_t)(baseIndex + i);
					data[2] = static_cast<uint8_t>(~data[1]);
					bus.sendPacket(1 << i, 255, data, 3);
					//bus.sendBroadcast(1 << i, data, 1);
					Delay_Ms(1000);
				}
				break;
			}
			case BUS_LED:
			{
				uint8_t addr = ser.getUint8();
				sendBlink(addr);
				break;
			}
			case BUS_LINES_STATE:
			{
				uint16_t lines = bus.getCMD();
				ser.writeUint8(3);
				ser.writeUint8(BUS_LINES_STATE);
				ser.writeUint8(lines & 0xff);
				ser.writeUint8((lines >> 8) & 0xff);
				ser.flush();
				break;
			}
			case BUS_PING:
			{
				pingMCUs();
				ser.writeUint8(MAX_MCUS + 1);
				ser.writeUint8(BUS_PING);
				for(int i = 0; i < MAX_MCUS; i++)
					ser.writeUint8(mcuStates[i]);
				ser.flush();
				break;
			}
			case BUS_RAYMARCHER_RENDER_PIXEL_RESULT:
			{
				pollResults();
				break;
			}
			case BUS_RAYMARCHER_RENDER_PIXEL:
			{
				static uint8_t id;
				id = ser.getUint8();
				uint8_t data[25];
				data[0] = BUS_RAYMARCHER_RENDER_PIXEL;
				//origin
				*(int32_t*)(&data[1]) = ser.readInt32();
				*(int32_t*)(&data[5]) = ser.readInt32();
				*(int32_t*)(&data[9]) = ser.readInt32();
				//direction
				*(int32_t*)(&data[13]) = ser.readInt32();
				*(int32_t*)(&data[17]) = ser.readInt32();
				*(int32_t*)(&data[21]) = ser.readInt32();
				if(bus.sendPacket(1 << (id & 0xf), id, data, 25) != HostBus::ERROR_SUCCESS) 
				{
					bus.sendReset(1 << (id & 0xf), id);						
					ser.writeUint8(2);
					ser.writeUint8(BUS_PACKET_LOST);
					ser.writeUint8(id);
					ser.flush();
					break;
				}
				mcuStates[id] = MCU_RAYMARCHING;
				mcuRenderResultSize[id] = 0;
				for(int i = 0; i < 3; i++)
					mcuRenderResult[id][i] = 0;
				break;				
				/*uint8_t id = prepareRenderPacket();
				//bus.resetCMD(1 << (id & 0xf));
				if(bus.sendPacket(1 << (id & 0xf), id, (uint8_t*)mcuRenderStart[id], 25) != HostBus::ERROR_SUCCESS) 
					break;
				mcuStates[id] = MCU_RAYMARCHING;
				mcuRenderResultSize[id] = 0;
				break;*/
			}
			default:
				break;
		}
    }
}
