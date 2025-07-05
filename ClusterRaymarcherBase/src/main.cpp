#include <ch32v20x.h>
#include <stdint.h>
//#include "systemFix.h"
#include "raymarcher/raymarcher.h"
//#include <debug.h>
#include "timer.h"
#include "gpio.h"

//#define HOST_PROGRAMMER

#ifdef HOST_PROGRAMMER

#include "usb/usb_serial.h"
USBSerial Serial;
#include "HostBusCH32V208.h"

#else 
#include "UARTSerial.h"
UARTSerial Serial;
#include "HostBusCH32V208Cluster.h"

#endif

bool getPacket(uint8_t *packet, uint8_t &packetLength)
{

//	uint32_t start = getTime();
	while(!Serial.available());
/*	{
		if(getTime() - start > ms2ticks(1000)) //100ms timeout
		{
			Serial.write(1); //send error packet
			Serial.write(BUS_HOST_HEARTBEAT);
			Serial.flush();
			return false;
		}
	}*/
	packetLength = Serial.read();
	if(!packetLength) return false;	//empty packet?
	for(int i = 0; i < packetLength; i++)
	{
		uint32_t start = getTime();
		while(Serial.available() == 0)
		{
			if(getTime() - start > bus.timeoutResponse) //100ms timeout
			{
				Serial.write(1); //send error packet
				Serial.write(BUS_HOST_ERROR);
				Serial.flush();
				return false;
			}
		}
		packet[i] = Serial.read(); //read data
	}
	return true;
}

//#include <debug.h>
int main(void)
{
	//SetSysClockTo144_HSIfix();
	//SystemCoreClockUpdate();


	initDelayTimer();
	Serial.begin(115200);
	#ifdef HOST_PROGRAMMER
		initGpio();
	#endif
	bus.init();
	delayMs(1000);
	
    while(1)
    {
		uint8_t packetLength = 0;
 		uint8_t packet[35];
		while(!getPacket(packet, packetLength));
		/*while(Serial.available() < 1);	//(length, [instruction,] [target,] [data..])
		uint8_t packetLength = Serial.read();
		if(!packetLength) continue;	//empty packet?
		while(Serial.available() < packetLength); //wait until data is complete
		uint8_t packet[35];
		for(int i = 0; i < packetLength; i++)
			packet[i] = Serial.read(); //read data
*/
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
				//short ack
				data[0] = 0;
				Serial.write(data, 1);
				/*data[0] = 3;
				data[1] = BUS_HOST_FORWARD;
				data[2] = mcu;
				data[3] = (uint8_t)size;
				Serial.write(data, 4); //send data*/
				break;
			}
			case BUS_HOST_BROADCAST:
			{
				uint16_t lines = *(uint16_t*)&(packet[1]);
				HostBus::ErrorCode err = bus.sendBroadcast(lines, &(packet[3]), packetLength - 3);
				uint8_t data[6];
				if(err != HostBus::ERROR_SUCCESS)
				{
					data[0] = 4;
					data[1] = BUS_HOST_ERROR;
					data[2] = (uint8_t)err;
					data[3] = 255;
					data[4] = (uint8_t)packetLength - 3;
					Serial.write(data, 5); //send error packet
					Serial.flush();
					break;
				}				
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
				//short ack if no data
				if(size == 0)
				{
					data[0] = 0;
					Serial.write(data, 1); //send data
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
			case BUS_HOST_GET_LINES:
			{
				uint16_t lines = bus.getCMD();
				uint8_t lines2 = (bus.getCLK() ? 0x01 : 0) |
					(bus.getACK() ? 0x02 : 0) |
					(bus.getFULL() ? 0x04 : 0) |
					(bus.getEOT() ? 0x08 : 0) |
					((uint8_t)bus.getType() << 4); 
				Serial.write(4);
				Serial.write(BUS_HOST_GET_LINES);
				Serial.write(lines & 0xff);
				Serial.write((lines >> 8) & 0xff);
				Serial.write(lines2);
				Serial.flush();
				break;
			}
			case BUS_HOST_SET_LINES:
			{
				bus.setCMD(*(uint16_t*)&(packet[1]));
				uint8_t signals = packet[3];
				if(signals & 0x01) bus.setCLK(); else bus.resetCLK();
				if(signals & 0x02) bus.setACK(); else bus.resetACK();
				if(signals & 0x04) bus.setFULL(); else bus.resetFULL();
				if(signals & 0x08) bus.setEOT(); else bus.resetEOT();
				bus.setType((HostBus::RequestType)((signals >> 4) & 0b11));
				Serial.write(1);
				Serial.write(BUS_HOST_SET_LINES);
				break;
			}
			case BUS_HOST_TIMINGS:
			{
				bus.timeoutSignalChange = us2ticks(*(uint32_t*)&(packet[1]));
				bus.timeoutResponse = us2ticks(*(uint32_t*)&(packet[5]));
				break;
			}
			default:
			{
				Serial.write(1);
				Serial.write(BUS_HOST_ERROR);
				break;
			}
		}
    }
}
