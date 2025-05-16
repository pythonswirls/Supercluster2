#include "usb/usb_serial.h"
#include"systemFix.h"
#include "raymarcher/raymarcher.h"
#include <debug.h>

USBSerial Serial;

void setup()
{
	SetSysClockTo144_HSIfix();
	SystemCoreClockUpdate();
	Serial.begin(115200);
	loadScene(0);
	Delay_Init();
}

void loop()
{
	static int i = 0;
	Serial.write((uint8_t*)&i, 4);
	i++;
	Serial.flush();
	Delay_Ms(100);
	//Serial.println("Hello World!");
	//delay(10);
}

int main()
{
	setup();
	while (true)
	{
		loop();
	}
	return 0;
}