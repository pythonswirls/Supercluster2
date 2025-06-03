#pragma once

enum BusInstruction
{
	BUS_RAYMARCHER_INIT = 0x40,
	BUS_RAYMARCHER_RENDER_PIXEL = 0x41,
	BUS_RAYMARCHER_RENDER_PIXEL_RESULT = 0x42,

	BUS_LED = 0xe0,
	BUS_SET_INDEX = 0xf0,
	BUS_GET_UID = 0xf1,
	BUS_MODE_IO0 = 0xf2,
	BUS_MODE_IO1 = 0xf3,
	BUS_WRITE_FLASH = 0xf4,
	BUS_JUMP_TO_FLASH = 0xf5,
	BUS_EXECUTE = 0xf6,
	BUS_HALT = 0xf87,
	BUS_PING = 0xf8,
	BUS_READ = 0xfe
};

////////// HAL /////////////

void initBus()
{
    uint32_t cfg = 0;
    for(int i = 0; i < 8; i++)
        cfg |= (0b0100 /*open drain*/ | 0b11 /*30MHz output*/) << (4 * i);
    GPIOC->OUTDR = 0xff;    //for open drain need to leave high for others to speak
    GPIOC->CFGLR = cfg;
    GPIOD->OUTDR = 0xff;
    GPIOD->CFGLR = cfg;
}

uint8_t readBusA()
{
    return GPIOC->INDR;
}

uint8_t readBusB()
{
    return GPIOD->INDR;
}

void writeBusA(uint8_t b)
{
    GPIOC->OUTDR = b;
}

void writeBusB(uint8_t b)
{
    GPIOD->OUTDR = b;
}

void clearBusA()
{
    GPIOC->OUTDR = 0xff;	//open drain mode
}

void clearBusB()
{
    GPIOD->OUTDR = 0xff;	//open drain mode
}

int getIo0()
{
	return (GPIOA->INDR >> 1) & 1;
}

int getIo1()
{
	return (GPIOA->INDR >> 2) & 1;
}

//////////////////////////////////////////////////

bool readBusByte(uint8_t id, uint8_t &b)
{
	while(getIo1())
	{
		//wait for this clock to pass
	}
	while(!getIo1())
	{
		//wait for nect clock
	}
	if(readBusA() != id) return false;
	b = readBusB();
	return true;
}

bool readBusInt32(uint8_t id, int32_t &i)
{
	uint8_t b;
	if(!readBusByte(id, b)) return false;
	i = b;
	if(!readBusByte(id, b)) return false;
	i |= ((uint32_t)b) << 8;
	if(!readBusByte(id, b)) return false;
	i |= ((uint32_t)b) << 16;
	if(!readBusByte(id, b)) return false;
	i |= ((uint32_t)b) << 24;
	return true;
}

volatile bool busWriting = 0;
volatile int busInBufferCurrent = 0;
volatile int busInBufferEnd = 0;
volatile int busOutBufferCurrent = 0;
volatile int busOutBufferEnd = 0;
volatile bool busInOverflow = false;
volatile bool busRead = false;
volatile uint8_t busId = 0;
volatile uint8_t busOutBuffer[32];
volatile uint8_t busInBuffer[32];
volatile uint8_t interruptCount = 0;

void setBusId(uint8_t id)
{
	busId = id;
}

bool writeBusByte(uint8_t b)
{
	int newEnd = (busOutBufferEnd + 1) & 31;
	if(newEnd == busOutBufferCurrent) return false;
	busOutBuffer[busOutBufferEnd] = b;
	busOutBufferEnd = newEnd;
	return true;
}

bool readBusByte(uint8_t &b)
{
	if(busInBufferCurrent == busInBufferEnd) return false;
	b = busInBuffer[busInBufferCurrent];
	busInBufferCurrent = (busInBufferCurrent + 1) & 31;
	return true;
}

void __attribute__((interrupt("WCH-Interrupt-fast"))) busClockInterrupt(void)
{
//    if(EXTI->INTFR & EXTI_Line2)
	{
		EXTI->INTFR = EXTI_Line2;
		if(GPIOA->INDR & 4) //rising edge
		{
			if(!busWriting)
			{
				//read new command
				if(readBusA() == busId)
				{
					uint8_t data = readBusB();
					if(data == BUS_READ)	//host request data
					{
						busRead = true;
						return;
					}
					//queue other instruction/data to process
					int end = (busInBufferEnd + 1) & 31;
					if(end == busInBufferCurrent) //buffer overflow
						busInOverflow = true;
					else
					{
						busInBuffer[busInBufferEnd] = data;
						busInBufferEnd = end;
					}
				}
			}
			else
			busWriting = false;
		}
		else
		{
			if(busOutBufferCurrent == busOutBufferEnd) //all data Sent
			{
				clearBusA();
				clearBusB();
			}
			else
			{
				writeBusA(busId);
				writeBusB(busOutBuffer[busOutBufferCurrent]);
				busOutBufferCurrent = (busOutBufferCurrent + 1) & 31;
			}
			//interruptCount++;
		}
	}
}



/*bool writeBusByte(uint8_t id, uint8_t b)
{
	while(getIo1())
	{
		//wait for this clock to pass
	}
	writeBusA(id);
	writeBusB(interruptCount);
	while(!getIo1())
	{
		//wait for next clock
	}
	while(getIo1())
	{
		//wait for this clock to pass
	}
	//clear the bus
	clearBusA();
	clearBusB();

	return true;
}*/

bool writeBusInt32(uint8_t id, int32_t i)
{
	if(!writeBusByte((i >>  0) & 0xff)) return false;
	if(!writeBusByte((i >>  8) & 0xff)) return false;
	if(!writeBusByte((i >> 16) & 0xff)) return false;
	if(!writeBusByte((i >> 24) & 0xff)) return false;
	return true;
}
