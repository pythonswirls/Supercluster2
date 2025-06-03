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

void setMcuIo1(int state, int bank = -1)
{
	if(BOARD_TYPE_PROGRAMMER)
	{
		if(bank != -1)
		{
			if(state)
				gndBank[bank]->BSHR = 1 << gndPin[bank];
			else
				gndBank[bank]->BCR = 1 << gndPin[bank];
		}
		else
		{
			if(state)
				for(int i = 0; i < 16; i++)
					gndBank[i]->BSHR = 1 << gndPin[i];
			else
				for(int i = 0; i < 16; i++)
					gndBank[i]->BCR = 1 << gndPin[i];
		}
	}
}

void clockDelay()
{
	//Delay_Us(1);
	Delay_Ms(1);
}

void setBusData(uint8_t addr, uint8_t data)
{
    GPIOB->OUTDR = (((uint16_t)data) << 8) | addr;
}

void getBusData(uint8_t &addr, uint8_t &data)
{
	uint16_t bus = GPIOB->INDR;
	addr = bus & 0xff;
	data = bus >> 8;
}


bool readBusByte(uint8_t id, uint8_t &b, int bank)
{
	setMcuIo1(0, bank);
	clockDelay();
	setMcuIo1(1, bank);
	uint8_t addr = 0;
	getBusData(addr, b);
	setMcuIo1(0, bank);
	if(addr != id) return false;
	return true;
}

bool readBusInt32(uint8_t id, int32_t &i, int bank)
{
	uint8_t b;
	if(!readBusByte(id, b, bank)) return false;
	i = b;
	if(!readBusByte(id, b, bank)) return false;
	i |= ((uint32_t)b) << 8;
	if(!readBusByte(id, b, bank)) return false;
	i |= ((uint32_t)b) << 16;
	if(!readBusByte(id, b, bank)) return false;
	i |= ((uint32_t)b) << 24;
	return true;
}

void sendBusPacket(uint8_t addr, int size, const uint8_t *data)
{
	int bank = addr & 15;//addr >> 4;
	if(addr == 255) bank = -1;
	for(int i = 0; i < size; i++)
	{
		setBusData(addr, data[i]);
		setMcuIo1(1, bank);
		clockDelay();
		setMcuIo1(0, bank);
		clockDelay();
	}
	setBusData(0xff, 0xff);
}

void sendBusPacket(uint8_t addr, uint8_t instr)
{
	int bank = addr & 15;//addr >> 4;
	if(addr == 255) bank = -1;
	setBusData(addr, instr);
	setMcuIo1(1, bank);
	clockDelay();
	setMcuIo1(0, bank);
	clockDelay();
	setBusData(0xff, 0xff);	
}