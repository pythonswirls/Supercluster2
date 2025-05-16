#include "usb/usb_serial.h"
#include"systemFix.h"
#include "raymarcher/raymarcher.h"

USBSerial Serial;

void setup()
{
	SetSysClockTo144_HSIfix();
	SystemCoreClockUpdate();
	Serial.begin(115200);
	loadScene(0);
}


void loop()
{

	Serial.println("Hello World!");
	//delay(1000);
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