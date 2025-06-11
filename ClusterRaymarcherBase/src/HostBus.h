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

	bool waitACK(bool value, uint16_t lines, int timeout)
	{
		int wait = timeout / 100; //100us per iteration
		while(getACK() != value) 
		{
			Delay_Us(100);
			wait--;
			if(wait <= 0) 
			{
				resetSignals(lines);
				return false; //timeout
			}
		}
		return true; //ACK received
	}

	bool waitEOT(bool value, uint16_t lines, int timeout)
	{
		int wait = timeout / 100; //100us per iteration
		while(getEOT() != value) 
		{
			Delay_Us(100);
			wait--;
			if(wait <= 0) 
			{
				resetSignals(lines);
				return false; //timeout
			}
		}
		return true; //ACK received
	}

	bool waitDATA(uint8_t value, uint16_t lines, int timeout)
	{
		int wait = timeout / 100; //100us per iteration
		while(getData() != value) 
		{
			Delay_Us(100);
			wait--;
			if(wait <= 0) 
			{
				resetSignals(lines);
				return false; //timeout
			}
		}
		return true; //ACK received
	}

	bool sendBroadcast(uint16_t lines, const uint8_t *data, int size, int signalDelayMicros = 1000)
	{
		setType(REQUEST_BROADCAST);
		setData(255);
		setCLK();
		setCMD(lines);
		Delay_Us(signalDelayMicros);
		resetCMD(lines);
		resetType();
		Delay_Us(signalDelayMicros);
		resetCLK();
		for(int i = 0; i < size; i++)
		{
			setData(data[i]);
			if(i == size - 1) 
				setEOT();
			setCLK();
			Delay_Us(signalDelayMicros);
			resetCLK();
			Delay_Us(signalDelayMicros);
		}
		resetData();
		resetEOT();
		return true;
	}

	enum ErrorCode
	{
		ERROR_SUCCESS = 0,
		ERROR_TIME_OUT_CMD_ACK = 1,
		ERROR_TIME_OUT_CMD_NACK = 2,
		ERROR_TIME_OUT_CMD_FULL_NACK = 3,
		ERROR_TIME_OUT_CMD_FULL = 4,
		ERROR_TIME_OUT_CLK_ACK = 5,
		ERROR_TIME_OUT_CLK_NACK = 6,
		ERROR_TIME_OUT_CLK_FULL_NACK = 7,
		ERROR_TIME_OUT_CLK_FULL = 8,
		ERROR_TIME_OUT_SET_DATA = 9,
		ERROR_TIME_OUT_SET_EOT = 10,
	};

	//send a packet. return true is successful, false if failed
	ErrorCode sendPacket(uint16_t lines, uint8_t id, const uint8_t *data, int size, int timeout = 100000)
	{
		setType(REQUEST_RECEIVE);
		setData(id);
		if(!waitDATA(id, lines, timeout))return ERROR_TIME_OUT_SET_DATA;
		setCLK();
		setCMD(lines);
		if(!waitACK(true, lines, timeout))return ERROR_TIME_OUT_CMD_ACK;
		resetCMD(lines);
		resetType();
		if(getFULL())	
		{
			setEOT(); //set end of transmission
			resetData();
			resetCLK();
			if(!waitACK(false, lines, timeout)) return ERROR_TIME_OUT_CMD_FULL_NACK;
			resetEOT();
			return ERROR_TIME_OUT_CMD_FULL;
		}
		resetCLK();
		if(!waitACK(false, lines, timeout)) return ERROR_TIME_OUT_CMD_NACK;
		//state = STATE_TRANSMIT;
		for(int i = 0; i < size; i++)
		{
			setData(data[i]);
			if(!waitDATA(data[i], lines, timeout))return ERROR_TIME_OUT_SET_DATA;
			if(i == size - 1)
			{ 
				setEOT();
				if(!waitEOT(true, lines, timeout)) return ERROR_TIME_OUT_SET_EOT;
			}
			setCLK();
			if(!waitACK(true, lines, timeout)) return ERROR_TIME_OUT_CLK_ACK;
			if(getFULL() && i < size - 1)	
			{
				setEOT(); //set end of transmission
				resetData();
				resetCLK();
				if(!waitACK(false, lines, timeout)) return ERROR_TIME_OUT_CLK_FULL_NACK;
				resetEOT();
				return ERROR_TIME_OUT_CLK_FULL;
			}
			resetCLK();
			if(!waitACK(false, lines, timeout)) return ERROR_TIME_OUT_CLK_NACK;
		}
		resetData();
		resetEOT();
		return ERROR_SUCCESS;
	}

	void sendReset(uint16_t lines)
	{
		setType(REQUEST_RESET);
		setData(id);
		setCLK();
		setCMD(lines);
		Delay_Ms(1);
		resetSignals(lines);
		Delay_Ms(1);
	}

	ErrorCode receivePacket(uint16_t lines, uint8_t id, uint8_t *data, int &size, int maxSize, int timeout = 100000)
	{
		setType(REQUEST_TRANSMIT);
		setData(id);
		setCLK();
		setCMD(lines);
		if(!waitACK(true, lines, timeout)) return ERROR_TIME_OUT_CMD_ACK;
		size = 0;
		if(getEOT())
		{
			resetSignals(lines);
			return ERROR_SUCCESS; //no data to receive
		}
		resetType();
		resetCMD(lines);
		resetCLK();
		resetData();
		if(!waitACK(false, lines, timeout)) return ERROR_TIME_OUT_CMD_NACK;

		while(size < maxSize)
		{
			setCLK();
			if(!waitACK(true, lines, timeout)) return ERROR_TIME_OUT_CLK_ACK; //timeout
			data[size++] = getData();
			bool end = getEOT();
			resetCLK();
			if(!waitACK(false, lines, timeout)) return ERROR_TIME_OUT_CLK_NACK; //timeout
			if(end) break;
		}
		resetSignals(lines);
		return ERROR_SUCCESS;
	}
};