#pragma once
#include "debug.h"
#include <stdint.h>

enum BusInstruction
{
	BUS_RAYMARCHER_INIT = 0x10,
	BUS_RAYMARCHER_RENDER_PIXEL = 0x11,
	BUS_RAYMARCHER_RENDER_PIXEL_RESULT = 0x12,
	BUS_RAYMARCHER_CAM_POS = 0x13,

	BUS_SHA256_START = 0x20,
	BUS_SHA256_END = 0x21,
	BUS_SHA256_COUNT = 0x22,

	BUS_LED = 0xd0,
	BUS_PING = 0xd1,
	
	BUS_CLIENT_RESET = 0xe0,		//reset client
	BUS_CLIENT_SET_INDEX = 0xe4,
	BUS_CLIENT_ERROR = 0xe5,		//error in client
	BUS_CLIENT_TIMINGS = 0xe7,	//set timeout for client

	BUS_HOST_RESET = 0xf0,
	BUS_HOST_FORWARD = 0xf1,		//forward packet to client
	BUS_HOST_BROADCAST = 0xf2,		//broadcast packet to flagged clients
	BUS_HOST_FETCH = 0xf3,			//fetch data from client

	BUS_HOST_ERROR = 0xf5,
	BUS_HOST_SUCCESS = 0xf6,
	BUS_HOST_TIMINGS = 0xf7,	//set timeout for host
	BUS_HOST_GET_LINES = 0xf8,
	BUS_HOST_SET_LINES = 0xf9,

};

template<int bufferSize = 32>		//needs to be power of two
class RingBuffer
{
	public:
	uint8_t buffer[bufferSize];
	uint32_t pos;
	uint32_t size;

	RingBuffer():
		pos(0),
		size(0)
	{
	}

	bool write(uint8_t data)
	{
		if(size + 1 >= bufferSize) return false;
		buffer[(pos + size) & (bufferSize - 1)] = data;
		size++;
		return true;
	}

	bool write(uint16_t data)
	{
		if(size + 2 >= bufferSize) return false;
		write((uint8_t)((data >>  0) & 0xff));
		write((uint8_t)((data >>  8) & 0xff));
		return true;
	}

	bool write(uint32_t data)
	{
		if(size + 4 >= bufferSize) return false;
		write((uint8_t)((data >>  0) & 0xff));
		write((uint8_t)((data >>  8) & 0xff));
		write((uint8_t)((data >> 16) & 0xff));
		write((uint8_t)((data >> 24) & 0xff));
		return true;
	}

	bool read(uint8_t &data)
	{
		if(size == 0) return false;
		data = buffer[pos];
		pos = (pos + 1) & (bufferSize - 1);
		size--;
		return true;
	}

	bool read(uint16_t &data)
	{
		if(size < 2) return false;
		uint8_t b;
		read(b);
		data = b;
		read(b);
		data |= ((uint16_t)b) << 8;
		return true;
	}

	bool read(int16_t &data)
	{
		uint8_t *d = (uint8_t*)(&data);
		if(size < 2) return false;
		read(d[0]);
		read(d[1]);
		return true;
	}

	bool read(uint32_t &data)
	{
		if(size < 4) return false;
		uint8_t b;
		read(b);
		data = b;
		read(b);
		data |= ((uint16_t)b) << 8;
		read(b);
		data |= ((uint16_t)b) << 16;
		read(b);
		data |= ((uint16_t)b) << 24;
		return true;
	}

	bool peek(uint8_t &data, int offset = 0)
	{
		if(size <= offset) return false;
		data = buffer[(this->pos + offset) & (bufferSize - 1)];
		return true;
	}

	int space()
	{
		return bufferSize - size;
	}

	void clear()
	{
		size = 0;
	}
};

class Bus
{
	public:

	enum State
	{
		STATE_IDLE = 0x00,
		STATE_RECEIVE = 0x01,
		STATE_TRANSMIT = 0x02,
		STATE_RECEIVE_BROADCAST = 0x03,
		STATE_ERROR = 0xff
	};

	enum RequestType
	{
		REQUEST_RESET = 0,
		REQUEST_RECEIVE = 1,
		REQUEST_TRANSMIT = 2,
		REQUEST_BROADCAST = 3,
		REQUEST_NONE = 3
	};

	enum ClientState
	{
		HOST_STATE_HAS_MORE_DATA = 0,
		HOST_STATE_END_OF_TRANSMISSION = 1, //last packet data bits
	};

	State state;
	uint8_t id;
	uint32_t timeoutSignalChange;
	uint32_t timeoutResponse;

	Bus()
		:state(STATE_IDLE), id(0)
	{
	}

	virtual bool initIo() = 0;

	bool init()
	{
		timeoutSignalChange = 100000; //100000us
		timeoutResponse = 100000; //100000us
		state = STATE_ERROR;
		if(initIo()) return false;
		state = STATE_IDLE;
		return true;
	}

	virtual void enableReceive() = 0;
	virtual void enableTransmit() = 0;
	virtual void disableReceive() = 0;
	virtual void disableTransmit() = 0;

	virtual void setCMD(uint16_t lines) = 0;
	virtual void resetCMD(uint16_t lines) = 0;
	virtual uint16_t getCMD() = 0;

	virtual void setCLK() = 0;
	virtual void resetCLK() = 0;
	virtual bool getCLK() = 0;

	virtual void setACK() = 0;
	virtual void resetACK() = 0;
	virtual bool getACK() = 0;

	virtual void setFULL() = 0;
	virtual void resetFULL() = 0;
	virtual bool getFULL() = 0;

	virtual void setEOT() = 0;
	virtual void resetEOT() = 0;
	virtual bool getEOT() = 0;

	virtual void setType(RequestType type) = 0;
	virtual RequestType getType() = 0;
	virtual void resetType() = 0;

	virtual void setData(uint8_t data) = 0;
	virtual uint8_t getData() = 0;
	virtual void resetData() = 0;

	void resetSignals(uint16_t lines)
	{
		resetCMD(lines);
		resetCLK();
		resetACK();
		resetFULL();
		resetEOT();
		resetType();
		resetData();
	}

	bool waitACK(bool value, uint16_t lines, int timeout)
	{
		uint32_t end = us2ticks(timeout);
		uint32_t t = getTime();
		while(getACK() != value) 
		{
			if(getTime() - t >= end) 
			{
				resetSignals(lines);
				return false; //timeout
			}
		}
		return true; //ACK set
	}

	bool waitEOT(bool value, uint16_t lines, int timeout)
	{
		uint32_t end = us2ticks(timeout);
		uint32_t t = getTime();
		while(getEOT() != value) 
		{
			if(getTime() - t >= end) 
			{
				resetSignals(lines);
				return false; //timeout
			}
		}
		return true; //EOT set
	}

	bool waitCLK(bool value, uint16_t lines, int timeout)
	{
		uint32_t end = us2ticks(timeout);
		uint32_t t = getTime();
		while(getCLK() != value) 
		{
			if(getTime() - t >= end) 
			{
				resetSignals(lines);
				return false; //timeout
			}
		}
		return true; //ACK set
	}

	bool waitDATA(uint8_t value, uint16_t lines, int timeout)
	{
		uint32_t end = us2ticks(timeout);
		uint32_t t = getTime();
		while(getData() != value) 
		{
			if(getTime() - t >= end) 
			{
				resetSignals(lines);
				return false; //timeout
			}
		}
		return true; //data correct
	}

	bool waitTYPE(Bus::RequestType type, uint16_t lines, int timeout)
	{
		uint32_t end = us2ticks(timeout);
		uint32_t t = getTime();
		while(getType() != type) 
		{
			if(getTime() - t >= end) 
			{
				resetSignals(lines);
				return false; //timeout
			}
		}
		return true; //data correct
	}

	bool waitCMD(uint16_t lines, int timeout)
	{
		uint32_t end = us2ticks(timeout);
		uint32_t t = getTime();
		while(getCMD() != lines) 
		{
			if(getTime() - t >= end) 
			{
				resetSignals(lines);
				return false; //timeout
			}
		}
		return true; //data correct
	}	

	virtual void debug(uint8_t data) {};

	virtual void processReceivedData() = 0;
};
