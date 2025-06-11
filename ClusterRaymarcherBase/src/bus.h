#pragma once
#include "debug.h"
#include <stdint.h>

enum BusInstruction
{
	BUS_RAYMARCHER_INIT = 0x40,
	BUS_RAYMARCHER_RENDER_PIXEL = 0x41,
	BUS_RAYMARCHER_RENDER_PIXEL_RESULT = 0x42,

	BUS_LED = 0xe0,
	BUS_LINES_STATE = 0xe1,
	BUS_SET_INDEX = 0xf0,
	BUS_GET_UID = 0xf1,
	BUS_MODE_IO0 = 0xf2,
	BUS_MODE_IO1 = 0xf3,
	BUS_WRITE_FLASH = 0xf4,
	BUS_JUMP_TO_FLASH = 0xf5,
	BUS_EXECUTE = 0xf6,
	BUS_HALT = 0xf7,
	BUS_PING = 0xf8,
	BUS_PACKET_LOST = 0xfe
};

template<int bufferSize = 32>	//needs to be power of two
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
		STATE_ERROR = 0xff
	};

	enum RequestType
	{
		REQUEST_RESET = 0,
		REQUEST_RECEIVE = 1,
		REQUEST_TRANSMIT = 2,
		REQUEST_BROADCAST = 3
	};

	enum ClientState
	{
		HOST_STATE_HAS_MORE_DATA = 0,
		HOST_STATE_END_OF_TRANSMISSION = 1, //last packet data bits
	};

	State state;
	uint8_t id;

	Bus()
		:state(STATE_IDLE), id(0)
	{
	}

	virtual bool initIo() = 0;

	bool init()
	{
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

	virtual void debug(uint8_t data) {};

	virtual void processReceivedData() = 0;
};
