constexpr bool BOARD_TYPE_PROGRAMMER = true;
constexpr bool BOARD_TYPE_BASE = false;

#include "usb/usb_serial.h"
#include "systemFix.h"
#include "raymarcher/raymarcher.h"
#include <debug.h>
#include "gpio.h"
#include "bus.h"
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
	sendBusPacket(addr, 1, data);
}

int main(void)
{
	SetSysClockTo144_HSIfix();
	SystemCoreClockUpdate();
	Delay_Init();
	Delay_Ms(100);
	Serial.begin(115200);
	initGpio();
	Delay_Ms(6000);
	initMCUs();

    while(1)
    {
		switch(ser.getUint8())
		{
			case BUS_SET_INDEX:
			{
				uint8_t baseIndex = ser.getUint8();
				for(int i = 0; i < 16; i++)
				{
					setBusData(baseIndex + i, BUS_SET_INDEX);
					setMcuIo1(1, i);
					Delay_Ms(200);
					//sendBusPacket(baseIndex + i, BUS_SET_INDEX);
					setMcuIo1(0, i);
				}
				break;
			}
			case BUS_LED:
			{
				uint8_t addr = ser.getUint8();
				sendBlink(addr);
				break;
			}
			case BUS_PING:
			{
				pingMCUs();
				ser.writeUint8(MAX_MCUS);
				for(int i = 0; i < MAX_MCUS; i++)
					ser.writeUint8(mcuStates[i]);
				ser.flush();
				break;
			}
			default:
				break;
		}
    }
}

////////////////////////////////////////////
/*
int main()
{
	SetSysClockTo144_HSIfix();
	SystemCoreClockUpdate();
	Delay_Init();
	Delay_Ms(100);
	Serial.begin(115200);
	//loadScene(0);


	while (true)
	{
		switch(ser.getUint8())
		{
			case BUS_SET_INDEX:
			{

				Vec3 pos(ser.readInt32(), ser.readInt32(), ser.readInt32());
				Vec3 dir(ser.readInt32(), ser.readInt32(), ser.readInt32());

				//Vec3 color = renderPixel(pos, dir, 2);

				ser.writeInt32(color.v[0]);
				ser.writeInt32(color.v[1]);
				ser.writeInt32(color.v[2]);
				ser.flush();
				break;
			}
			default:
				break;
		}
	}
	return 0;
}
*/