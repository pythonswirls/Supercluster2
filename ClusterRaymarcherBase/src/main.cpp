constexpr bool BOARD_TYPE_PROGRAMMER = true;
constexpr bool BOARD_TYPE_BASE = false;

#include "usb/usb_serial.h"
#include "systemFix.h"
#include "raymarcher/raymarcher.h"
//#include <debug.h>
#include "timer.h"
#include "gpio.h"
#include "HostBusCH32V208.h"
USBSerial Serial;

/// old programmer //////////////////////////////////////

int main(void)
{
	SetSysClockTo144_HSIfix();
	SystemCoreClockUpdate();
	initDelayTimer();
	delayMs(100);
	Serial.begin(115200);
	initGpio();
	bus.init();
	delayMs(6000);

    while(1)
    {
		while(Serial.available() < 1);	//(length, [instruction,] [target,] [data..])
		uint8_t packetLength = Serial.read();
		if(!packetLength) continue;	//empty packet?
		while(Serial.available() < packetLength); //wait until data is complete
		uint8_t packet[35];
		for(int i = 0; i < packetLength; i++)
			packet[i] = Serial.read(); //read data
		uint8_t instruction = packet[0];
		switch(instruction)	
		{
			case BUS_HOST_RESET:
			{
				bus.sendReset(0xffff); //reset all lines
				bus.resetSignals(0xffff); //reset all lines
				uint8_t data[2];
				data[0] = 1;
				data[1] = BUS_HOST_RESET;
				Serial.write(data, 2); //send data
				Serial.flush();
				break;
			}
			case BUS_HOST_FORWARD:
			{
				uint8_t mcu = packet[1];
				int size = 0;
				HostBus::ErrorCode err = bus.sendPacket(1 << (mcu & 0xf), mcu, &(packet[2]), packetLength - 2);
				uint8_t data[5];
				if(err != HostBus::ERROR_SUCCESS)
				{
					data[0] = 4;
					data[1] = BUS_HOST_ERROR;
					data[2] = (uint8_t)err;
					data[3] = mcu;
					data[4] = (uint8_t)size;
					Serial.write(data, 5); //send error packet
					Serial.flush();
					break;
				}
				data[0] = 3;
				data[1] = BUS_HOST_FORWARD;
				data[2] = mcu;
				data[3] = (uint8_t)size;
				Serial.write(data, 4); //send data
				Serial.flush();
				break;
			}
			case BUS_HOST_BROADCAST:
			{
				uint16_t lines = *(uint16_t*)&(packet[1]);
				//uint8_t led = BUS_LED;
//				HostBus::ErrorCode err = bus.sendPacket(lines, 0xff,  &(packet[3]), packetLength - 3);
				bus.sendBroadcast(lines, &(packet[3]), packetLength - 3);
				//bus.sendBroadcast(0xffff, &led, 1);
				uint8_t data[6];
				data[0] = 5;
				data[1] = BUS_HOST_BROADCAST;
				data[2] = packet[3];
				data[3] = packetLength - 3;
				data[4] = lines & 0xff;
				data[5] = lines >> 8;
				Serial.write(data, 6); //send data
				Serial.flush();
				break;
			}
			case BUS_HOST_FETCH:
			{
				uint8_t mcu = packet[1];
				uint8_t data[35];
				int size = 0;
				HostBus::ErrorCode err = bus.receivePacket(1 << (mcu & 0xf), mcu, &(data[3]), size, 32);
				if(err != HostBus::ERROR_SUCCESS)
				{
					data[0] = 4;
					data[1] = BUS_HOST_ERROR;
					data[2] = (uint8_t)err;
					data[3] = mcu;
					data[4] = (uint8_t)size;
					Serial.write(data, 5); //send error packet
					Serial.flush();
					break;
				}
				data[0] = (uint8_t)(size + 2);
				data[1] = BUS_HOST_FETCH;
				data[2] = mcu;
				Serial.write(data, size + 3); //send data
				Serial.flush();
				break;
			}
			case BUS_HOST_LINES_STATE:
			{
				uint16_t lines = bus.getCMD();
				uint8_t lines2 = (bus.getCLK() ? 0 : 0x01) |
					(bus.getACK() ? 0 : 0x02) |
					(bus.getFULL() ? 0: 0x04) |
					(bus.getEOT() ? 0: 0x08);
				Serial.write(4);
				Serial.write(BUS_HOST_LINES_STATE);
				Serial.write(lines & 0xff);
				Serial.write((lines >> 8) & 0xff);
				Serial.write(lines2);
				Serial.flush();
				break;
			}
			default:
			{
				break;
			}
		}
    }
}
