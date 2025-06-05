#pragma once
#include "Bus.h"
#include "debug.h"
#include <stdint.h>

class HostBus: public Bus
{
	public:
	HostBus()
		:Bus()
	{
	}

	//send a packet. return true is successful, false if failed
	bool sendPacket(uint8_t id, const uint8_t *data, int size, int timeout = 100)
	{
		//if(state != STATE_IDLE) return;
		//if(size == 0) return; //nothing to send
		resetEOT();	//if didn't wait for ACK
		////////////////request to client////////////////
		setType(REQUEST_RECIEVE);
		setData(id);
		setCLK();
		setCMD(id);
		int wait = timeout * 10;
		while(!getREADY()) 
		{
			Delay_Us(100);
			wait--;
			if(wait <= 0) 
			{
				resetSignals(id);
				return false; //timeout
			}
		}
		resetCMD(id);
		resetType();
		if(getFULL())	
		{
			setEOT(); //set end of transmission
			resetData();
			resetCLK();
			wait = timeout * 10;
			while(getREADY())
			{
				Delay_Us(100);
				wait--;
				if(wait <= 0) 
				{
					resetSignals(id);
					return false; //timeout
				}
			}
			resetEOT();
			//error, buffer is full
			return false;
		}
		resetCLK();
		wait = timeout * 10;
		while(getREADY())
		{
			Delay_Us(100);
			wait--;
			if(wait <= 0) 
			{
				resetSignals(id);
				return false; //timeout
			}
		}
		//state = STATE_TRANSMIT;
		for(int i = 0; i < size; i++)
		{
			setData(data[i]);
			if(i == size - 1) 
				setEOT();
			setCLK();
			while(!getREADY());	//TODO: timeout
			if(getFULL() && i < size - 1)	
			{
				setEOT(); //set end of transmission
				resetData();
				resetCLK();
				wait = timeout * 10;
				while(getREADY())			
				{
					Delay_Us(100);
					wait--;
					if(wait <= 0) 
					{
						resetSignals(id);
						return false; //timeout
					}
				}
				resetEOT();
				//error, buffer is full
				return false;
			}
			resetCLK();
			wait = timeout * 10;
			while(getREADY())			
			{
				Delay_Us(100);
				wait--;
				if(wait <= 0) 
				{
					resetSignals(id);
					return false; //timeout
				}
			}
		}
		resetData();
		resetEOT();
		return true;
	}
};