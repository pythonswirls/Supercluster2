#pragma once

enum MCUState
{
	MCU_NA = 0,
	MCU_IDLE = 1,
	MCU_RAYMARCHING = 2,
	MCU_BUSY = 255,
};

constexpr int MAX_MCUS = 160; //255 is broadcast
//MCUState 
volatile uint8_t mcuStates[MAX_MCUS];

void pingMCUs()
{
	/*
	sendBusPacket(0, BUS_PING);
	for(int i = 0; i < MAX_MCUS; i++)
	{
		uint8_t b = 0;
		if(!readBusByte(i, b, 0));
		mcuStates[i] = b;
	}
	return;
	for(int i = 0; i < MAX_MCUS; i++)
	{
		sendBusPacket(i, BUS_PING);
		uint8_t b = 0;
		int bank = i & 0xf;
		if(!readBusByte(i, b, bank));
		mcuStates[i] = b;
		/*
		b = 255;
		if(b == i)
			mcuStates[i] = MCU_IDLE;
		else
			mcuStates[i] = MCU_BUSY;*/
	/*}
	*/
}

void initMCUs()
{
	for(int i = 0; i < MAX_MCUS; i++)
		mcuStates[i] = MCU_NA;
}
