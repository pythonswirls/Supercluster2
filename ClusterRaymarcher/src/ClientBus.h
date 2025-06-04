#pragma	once
#include <stdint.h>
#include <ch32v00x.h>
#include "Bus.h"

class ClientBus: public Bus
{
	public:
	ClientBus()
		:Bus()
	{
	}

	void handleRequest()
	{
		debug(1);
		if(state != STATE_IDLE)
		{
			//error
			if(state == STATE_RECEIVE) disableReceive();
			if(state == STATE_TRANSMIT) disableTransmit();
			state = STATE_ERROR;
			return;
		}
		RequestType type = getType();
		switch(type)
		{
			case REQUEST_RECIEVE:
				//if(getData() != id) break;
			case REQUEST_BROADCAST:
				enableReceive();
				if(inBuffer.space() == 0) setFULL();
				state = STATE_RECEIVE;
				setREADY();
				break;
			case REQUEST_TRANSMIT:
				enableTransmit();
				state = STATE_TRANSMIT;
				if(outBuffer.size == 0) setEOT();
				setREADY();
				break;
			case REQUEST_RESET:
				break;
		}
	}

	void handleDataClockHigh()
	{
		resetREADY();
	}

	void handleDataClockLow()
	{
		if(state == STATE_RECEIVE)
		{
			if(getEOT())
			{
				//end of transmission
				disableReceive();
				resetREADY();
				resetFULL();
				processReceivedData();
				state = STATE_IDLE;
				return;
			}
			uint8_t data = getData();
			inBuffer.write(data);
			if(inBuffer.space() == 0)
				setFULL();
			setREADY();
		}
		else
		if(state == STATE_TRANSMIT)
		{
			if(getEOT())
			{
				//end of transmission
				disableTransmit();
				resetEOT();
				resetData();
				resetREADY();
				state = STATE_IDLE;
				return;
			}
			uint8_t data;
			outBuffer.read(data);
			setData(data);
			if(outBuffer.size == 0) setEOT();
			setREADY();
		}
		else
		{
			state = STATE_ERROR; //error, not in receive or transmit state
			return;
		}
	}

	virtual void setCMD(uint8_t id) {};
	virtual void resetCMD(uint8_t id) {};
	virtual void setCLK() {};
	virtual void resetCLK() {};
	virtual void resetType() {};
};
