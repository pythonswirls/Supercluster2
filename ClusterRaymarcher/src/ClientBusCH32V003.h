#pragma once
#include "ClientBus.h"
#include <stdint.h>
#include <ch32v00x.h>
#include "utils.h"

void  busInterrupt(void) __attribute__((interrupt("WCH-Interrupt-fast")));

class ClientBusCH32V003: public ClientBus
{
	public:
	ClientBusCH32V003()
		:ClientBus()
	{
	}

	virtual bool initIo()
	{
		//initialize GPIOs
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	
		uint32_t cfg = 0;
		for(int i = 0; i < 8; i++)
			cfg |= (0b0100 /*open drain*/ | 0b11 /*30MHz output*/) << (4 * i);
		GPIOC->OUTDR = 0b11111111;    //for open drain need to leave high for others to speak
		GPIOC->CFGLR = cfg;
		cfg = 0;
		for(int i = 0; i < 8; i++)
			if(i == 1 || i == 7) //avoid D1 and D7 to have swio still active
				cfg |= (0b0100 /*floating*/ | 0b00 /*input*/) << (4 * i);
			else
				cfg |= (0b0100 /*open drain*/ | 0b11 /*30MHz output*/) << (4 * i);

		GPIOD->OUTDR = 0b01111101;
		GPIOD->CFGLR = cfg;

		GPIOA->CFGLR = (GPIOA->CFGLR & 0b000011110000) |  ((0b0100 /*floating*/ | 0b00 /*input*/) << (4 * 2));
	
		//initialize interrupts
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

		//GPIO A2 interrupt
		AFIO->EXTICR = (AFIO->EXTICR & ~AFIO_EXTICR1_EXTI2) | AFIO_EXTICR1_EXTI2_PA;	//line 2 on PA2
		EXTI->INTENR |= EXTI_INTENR_MR2; // Enable EXT7 on line 2
		//EXTI->RTENR |= EXTI_RTENR_TR2;  // Rising edge trigger
		EXTI->FTENR |= EXTI_FTENR_TR2;  // Falling edge trigger	

		//GPIO D0 interrupt prepare
		AFIO->EXTICR = (AFIO->EXTICR & ~AFIO_EXTICR1_EXTI0) | AFIO_EXTICR1_EXTI0_PD;	//line 0 on PD0
		//EXTI->INTENR |= EXTI_INTENR_MR0; // Enable EXT7 on line 0
		EXTI->RTENR |= EXTI_RTENR_TR0;  // Rising edge trigger
		EXTI->FTENR |= EXTI_FTENR_TR0;  // Falling edge trigger	
	
		SetVTFIRQ((uint32_t)busInterrupt, EXTI7_0_IRQn, 0, ENABLE);
		NVIC_EnableIRQ(EXTI7_0_IRQn);
	
		return true;
	}

	virtual void enableReceive()
	{
		EXTI->INTENR |= EXTI_INTENR_MR0; // Enable EXT7 on line 0
	}

	virtual void enableTransmit()
	{
		EXTI->INTENR |= EXTI_INTENR_MR0; // Enable EXT7 on line 0
	}

	virtual void disableReceive()
	{
		EXTI->INTENR &= ~EXTI_INTENR_MR0; 	// Disable EXT7 on line 0
		GPIOD->BSHR = 0b01111101;			//release open drain lines, avoid D1 and D7 here to have swio still active
	}

	virtual void disableTransmit()
	{
		EXTI->INTENR &= ~EXTI_INTENR_MR0; 	// Disable EXT7 on line 0
		GPIOC->BSHR = 0b11111111; 			//release open drain lines
		GPIOD->BSHR = 0b01111101;			//release open drain lines, avoid D1 and D7 here to have swio still active
	}

	virtual void setACK() 
	{
		GPIOD->BCR = 0b00010000; //set PD4 low, ready to receive or transmit
	}

	virtual void resetACK()
	{
		GPIOD->BSHR = 0b00010000; //set PD4 high, not ready
	}

	virtual bool getACK()
	{
		return (GPIOD->INDR & 0b00010000) == 0;
	}

	virtual void setFULL()
	{
		GPIOD->BCR = 0b00100000; //set PD5 low, ready to receive or transmit
	}

	virtual void resetFULL()
	{
		GPIOD->BSHR = 0b00100000; //set PD5 low, ready to receive or transmit
	}

	virtual bool getFULL()
	{
		return (GPIOD->INDR & 0b00100000) == 0; //get PD5
	}

	virtual void setEOT() 
	{
		GPIOD->BCR = 0b01000000; //set PD6 low, end of transmission
	}

	virtual void resetEOT()
	{
		GPIOD->BSHR = 0b01000000; //set PD6 high, not end of transmission
	}
	
	virtual bool getEOT()
	{
		return (GPIOD->INDR & 0b01000000) == 0; //get PD6
	}
	
	virtual void setType(RequestType type)
	{
		GPIOD->BCR = 0b00001100; //set PD2 and PD3 low, request type
		GPIOD->BSHR = (uint32_t)type << 2; //set PD2 and PD3 high, request type
	}

	virtual RequestType getType()
	{
		return (RequestType)((GPIOD->INDR & 0b00001100) >> 2); //get PD2 and PD3
	}

	virtual void setData(uint8_t data)
	{
		GPIOC->OUTDR = (uint32_t)data; //set PC0-PC7 data
	}

	virtual uint8_t getData()
	{
		return (uint8_t)(GPIOC->INDR & 0xff); //get PC0-PC7 data
	}

	virtual void resetData() 
	{
		GPIOC->OUTDR = 0b11111111; //set PC0-PC7 high, reset data
	};

	virtual void processReceivedData()
	{
		//copy packet trigger processing
	}

	virtual bool getCLK()
	{
		return (GPIOD->INDR & 0b00000001) == 0; //get PD0
	}

	virtual void debug(uint8_t data) {
		for(int i = 0; i < data; i++)
			blink();
	};
};

ClientBusCH32V003 bus;
void __attribute__((interrupt("WCH-Interrupt-fast"))) busInterrupt(void)
{
    if(EXTI->INTFR & EXTI_Line2)
	{
		bus.handleRequest();
		EXTI->INTFR = EXTI_Line2;
	}
	if(EXTI->INTFR & EXTI_Line0)
	{
		if(!bus.getCLK()) //PD0 is high
			bus.handleDataClockHigh();
		else
			bus.handleDataClockLow();
		EXTI->INTFR = EXTI_Line0;
	}
}

