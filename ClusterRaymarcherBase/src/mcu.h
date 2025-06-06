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
	uint8_t data[] = {BUS_PING};
	bus.sendBroadcast(0xffff, data, 1);
	Delay_Ms(100);	
	for(int i = 0; i < MAX_MCUS; i++)
	{
		uint8_t ping = 0;
		int size = 0;
		HostBus::ErrorCode code = bus.receivePacket(1 << (i & 0xf), i, &ping, size, 1);
		if(code == HostBus::ERROR_SUCCESS && size == 1 && ping == BUS_PING)
		{
			mcuStates[i] = MCU_IDLE;
			continue;
		}
	}
}

void initMCUs()
{
	for(int i = 0; i < MAX_MCUS; i++)
		mcuStates[i] = MCU_NA;
}
