#pragma once
#include "HostBus.h"
#include <stdint.h>
#include <ch32v20x.h>

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
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
		//16 CMD lines for MCUs
		GPIOC->OUTDR = 0xffff;
		GPIOC->CFGLR = 0x33333333; //set PC0-PC7 to push pull output, 50MHz
		GPIOC->CFGHR = 0x44433333; //set PC8-PC15 to push pull output, 50MHz
		GPIOB->CFGLR = 0x33333333;

		uint32_t cfg = 0;
		for(int i = 0; i < 8; i++)
			cfg |= (0b0100 /*open drain*/ | 0b11 /*50MHz output*/) << (4 * i);
		GPIOA->BSHR = 0xff;    //for open drain need to leave high for others to speak
		GPIOA->CFGLR = cfg;
		
		cfg = 0;
		for(int i = 0; i < 8; i++)
			if(i == 1 || i == 7) //avoid D1 and D7 to have swio still active
				cfg |= (0b0100 /*floating*/ | 0b00 /*input*/) << (4 * i);
			else
				cfg |= (0b0100 /*open drain*/ | 0b11 /*30MHz output*/) << (4 * i);
		GPIOB->BSHR = 0x7d00;
		GPIOB->BCR  = 0x8200;
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
		//TODO one write
		GPIOB->BCR = 0b0000110000000000; //set PB10 and PB11 low, request type
		GPIOB->BSHR = (uint32_t)type << 10; //set PB10 and PB11 to type
	}

	virtual void resetType()
	{
		GPIOB->BSHR = 0b0000110000000000; //set PB10 and PB11 high, reset request type
	}

	virtual RequestType getType()
	{
		return (RequestType)((GPIOD->INDR >> 10) & 0b11); //get PB10 and PB11, request type
	}

	virtual void setData(uint8_t data)
	{
		GPIOA->BCR = 0b11111111; 
		GPIOA->BSHR = (uint32_t)data;
	}

	virtual uint8_t getData()
	{
		return (uint8_t)(GPIOA->INDR & 0xff);
	}

	virtual void resetData() 
	{
		GPIOA->BSHR = 0b11111111; //set PB0-PB7 high, reset data
	};

	virtual void setCMD(uint16_t lines)
	{
		GPIOC->BCR = lines;
		GPIOB->BCR = lines >> 13;
	}

	virtual void resetCMD(uint16_t lines)
	{
		GPIOC->BSHR = lines;
		GPIOB->BSHR = lines >> 13;
	}

	virtual uint16_t getCMD()
	{
		return (GPIOC->INDR & 0xffff)^0xffff; 
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
