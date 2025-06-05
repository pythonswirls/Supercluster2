#pragma once
#include "HostBus.h"
#include <stdint.h>
#include <ch32v20x.h>

GPIO_TypeDef *mcuBank[] =  {GPIOA, GPIOA, GPIOA, GPIOD, GPIOA, GPIOA, GPIOA, GPIOD,
	GPIOA, GPIOA, GPIOA, GPIOD, GPIOA, GPIOD, GPIOD, GPIOA,};
const int mcuPin[] = {8, 4, 0, 3, 1, 5, 9, 4, 2, 6, 10, 5, 3, 6, 2, 7,};

class HostBusCH32V208: public HostBus
{
	public:
	HostBusCH32V208()
		:HostBus()
	{
	}

	virtual bool initIo()
	{
		//initialize GPIOs
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	
		//16 CMD lines for MCUs
		for(int i = 0; i < 16; i++)
		{
			mcuBank[i]->BSHR = 1 << mcuPin[i]; //set all pins high
			if(mcuPin[i] < 8)
				mcuBank[i]->CFGLR = (GPIOA->CFGLR & ~(0b1111 << (mcuPin[i] * 4))) | 
					((0b0000 /*push pull*/ | 0b11 /*output*/) << (mcuPin[i] * 4));
			else
				mcuBank[i]->CFGHR = (GPIOA->CFGLR & ~(0b1111 << ((mcuPin[i] - 8) * 4))) | 
					((0b0000 /*push pull*/ | 0b11 /*output*/) << ((mcuPin[i] - 8) * 4));
		}
	
		uint32_t cfg = 0;
		for(int i = 0; i < 8; i++)
			cfg |= (0b0100 /*open drain*/ | 0b11 /*50MHz output*/) << (4 * i);
		GPIOB->OUTDR = 0b0111110111111111;    //for open drain need to leave high for others to speak
		GPIOB->CFGLR = cfg;
		cfg = 0;
		for(int i = 0; i < 8; i++)
			if(i == 1 || i == 7) //avoid D1 and D7 to have swio still active
				cfg |= (0b0100 /*floating*/ | 0b00 /*input*/) << (4 * i);
			else
				cfg |= (0b0100 /*open drain*/ | 0b11 /*30MHz output*/) << (4 * i);
		GPIOB->CFGHR = cfg;
		return true;
	}

	virtual void enableReceive()
	{
	}

	virtual void enableTransmit()
	{
	}

	virtual void disableReceive()
	{
	}

	virtual void disableTransmit()
	{
	}

	virtual void setACK() 
	{
		GPIOB->BCR = 0b0001000000000000; 	//set PB12 low, ready to receive or transmit
	}

	virtual void resetACK()
	{
		GPIOB->BSHR = 0b0001000000000000; 	//set PB12 high, reset ready to receive or transmit
	}

	virtual bool getACK()
	{
		return (GPIOB->INDR & 0b0001000000000000) == 0;	 //get PB12
	}

	virtual void setFULL()
	{
		GPIOB->BCR = 0b0010000000000000; //set PB13 low, ready to receive or transmit
	}

	virtual void resetFULL()
	{
		GPIOB->BSHR = 0b0010000000000000; //set PB13 low, ready to receive or transmit
	}

	virtual bool getFULL()
	{
		return (GPIOB->INDR & 0b0010000000000000) == 0; //get PB13
	}

	virtual void setEOT() 
	{
		GPIOB->BCR = 0b0100000000000000; //set PB14 low, end of transmission
	}

	virtual void resetEOT()
	{
		GPIOB->BSHR = 0b0100000000000000; 
	}
	
	virtual bool getEOT()
	{
		return (GPIOB->INDR & 0b0100000000000000) == 0; 	//get PB14
	}
	
	virtual void setType(RequestType type)
	{
		GPIOB->BCR = 0b0000110000000000; //set PB10 and PB11 low, request type
		GPIOB->BSHR = (uint32_t)type << 10; //set PB10 and PB11 to type
	}

	virtual void resetType()
	{
		GPIOB->BSHR = 0b0000110000000000; //set PB10 and PB11 high, reset request type
	}

	virtual RequestType getType()
	{
		return (RequestType)((GPIOD->INDR & 0b0000110000000000) >> 10); //get PB10 and PB11, request type
	}

	virtual void setData(uint8_t data)
	{
		GPIOB->BCR = 0b0000000011111111; //set PB0-PB7 low, data
		GPIOB->BSHR = (uint32_t)data; //set PB0-PB7 data
	}

	virtual uint8_t getData()
	{
		return (uint8_t)(GPIOB->INDR & 0xff); //get PB0-PB7, data
	}

	virtual void resetData() 
	{
		GPIOB->BSHR = 0b0000000011111111; //set PB0-PB7 high, reset data
	};

	virtual void setCMD(uint8_t id)
	{
		//TODO breadcast
//		for(int i = 0; i < 16; i++)
//			mcuBank[i]->BCR = 1 << mcuPin[i];
		mcuBank[id & 0xf]->BCR = 1 << mcuPin[id & 0xf];
	}

	virtual void resetCMD(uint8_t id)
	{
//		for(int i = 0; i < 16; i++)
//			mcuBank[i]->BSHR = 1 << mcuPin[i];
		mcuBank[id & 0xf]->BSHR = 1 << mcuPin[id & 0xf];
	}

	virtual bool getCLK()
	{
		return (GPIOB->INDR & 0b0000000100000000) == 0; //get PB8
	}

	virtual void setCLK()
	{
		GPIOB->BCR = 0b0000000100000000; //set PB8 low, reset clock
	}

	virtual void resetCLK()
	{
		GPIOB->BSHR = 0b0000000100000000; //set PB8 high, reset clock
	}

	virtual void processReceivedData()
	{
		//not needed
	}
};

HostBusCH32V208 bus;
