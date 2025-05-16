#include "usb/usb_serial.h"
#include "systemFix.h"
#include "raymarcher/raymarcher.h"
#include <debug.h>

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

int main()
{
	SetSysClockTo144_HSIfix();
	SystemCoreClockUpdate();
	Delay_Init();
	Delay_Ms(100);
	Serial.begin(115200);
	loadScene(0);
	while (true)
	{
		switch(ser.getUint8())
		{
			case 0x69:
			{
				Vec3 pos(ser.readInt32(), ser.readInt32(), ser.readInt32());
				Vec3 dir(ser.readInt32(), ser.readInt32(), ser.readInt32());
				Vec3 color = renderPixel(pos, dir, 2);
				ser.writeInt32(color.v[0]);
				ser.writeInt32(color.v[1]);
				ser.writeInt32(color.v[2]);
				ser.flush();
				break;
			}
			default:
				break;
		}
		/*static int i = 0;
		Serial.write((uint8_t*)&i, 4);
		Serial.available();
		i++;
		Serial.flush();
		Delay_Ms(100);*/
	}
	return 0;
}