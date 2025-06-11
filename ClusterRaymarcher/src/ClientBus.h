#pragma	once
#include <stdint.h>
#include <ch32v00x.h>
#include "Bus.h"

class ClientBus: public Bus
{
	public:
	RingBuffer<> inBuffer;
	RingBuffer<> outBuffer;

	ClientBus()
		:Bus()
	{
	}

	void handleRequest()
	{
		RequestType type = getType();
		if(state != STATE_IDLE || type == REQUEST_RESET)
		{
			resetSignals(0);
			if(state == STATE_RECEIVE) disableReceive();
			if(state == STATE_TRANSMIT) disableTransmit();
			state = STATE_IDLE;
			//return;
		}
		switch(type)
		{
			case REQUEST_RECEIVE:
				{
					uint8_t data = getData();
					if(data != 255 && data != id) break;
				}
			case REQUEST_BROADCAST:
				enableReceive();
				if(inBuffer.space() == 0) setFULL();
				state = STATE_RECEIVE;
				setACK();
				break;
			case REQUEST_TRANSMIT:
				if(getData() != id) break;
				enableTransmit();
				state = STATE_TRANSMIT;
				if(outBuffer.size == 0) setEOT();
				setACK();
				break;
			case REQUEST_RESET:
				break;
		}
	}

	void handleDataClockHigh()
	{
		//debug(1);
		resetACK();
		if(getEOT())
		{
			//end of transmission
			if(state == STATE_RECEIVE)
			{
				disableReceive();
				resetFULL();
				processReceivedData();
				resetACK();
				state = STATE_IDLE;
			}
			else if(state == STATE_TRANSMIT)
			{
				disableTransmit();
				resetEOT();
				resetData();
				resetACK();
				state = STATE_IDLE;
			}
		}
	}

	void handleDataClockLow()
	{
		if(state == STATE_RECEIVE)
		{
			uint8_t data = getData();
			inBuffer.write(data);
			if(inBuffer.space() == 0)
				setFULL();
			setACK();
		}
		else
		if(state == STATE_TRANSMIT)
		{
			uint8_t data;
			outBuffer.read(data);
			setData(data);
			if(outBuffer.size == 0) setEOT();
			setACK();
		}
		else
		{
			state = STATE_ERROR; //error, not in receive or transmit state
			return;
		}
	}

	virtual void setCMD(uint16_t lines) {};
	virtual void resetCMD(uint16_t lines) {};
	virtual void setCLK() {};
	virtual void resetCLK() {};
	virtual void resetType() {};
};
