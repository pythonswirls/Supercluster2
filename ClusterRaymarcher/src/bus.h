#pragma once

enum BusInstruction
{
	BUS_RAYMARCHER_INIT = 0x40,
	BUS_RAYMARCHER_RENDER_PIXEL = 0x41,
	BUS_RAYMARCHER_RENDER_PIXEL_RESULT = 0x42,

    BUS_SET_INDEX = 0xf0,
    BUS_GET_UID = 0xf1,
    BUS_MODE_IO0 = 0xf2,
    BUS_MODE_IO1 = 0xf3,
    BUS_WRITE_FLASH = 0xf4,
    BUS_JUMP_TO_FLASH = 0xf5,
    BUS_EXECUTE = 0xf6,
    BUS_BLINK = 0xfe
};

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
	return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) ? 1 : 0;
}

int getIo1()
{
	return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) ? 1 : 0;
}

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

bool writeBusByte(uint8_t id, uint8_t b)
{
	while(getIo1())
	{
		//wait for this clock to pass
	}
	writeBusA(id);
	writeBusB(b);
	while(!getIo1())
	{
		//wait for nect clock
	}
	while(getIo1())
	{
		//wait for this clock to pass
	}
	//clear the bus
	clearBusA();
	clearBusB();

	return true;
}

bool writeBusInt32(uint8_t id, int32_t i)
{
	if(!writeBusByte(id, (i >>  0) & 0xff)) return false;
	if(!writeBusByte(id, (i >>  8) & 0xff)) return false;
	if(!writeBusByte(id, (i >> 16) & 0xff)) return false;
	if(!writeBusByte(id, (i >> 24) & 0xff)) return false;
	return true;
}
