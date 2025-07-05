#pragma once
#include <stdint.h>
#include <ch32v20x.h>
#include <string.h>

const uint8_t SERIAL_8N1 = 0x00;

class UARTSerial
{
public:
    void begin(uint32_t baud, uint8_t config = SERIAL_8N1, uint8_t timeout = 30)
    {
        _baud = baud;
        _config = config;
        _stop_bit = _config & 0x08 ? 2 : 1;
        _parity_bit = _config >> 4;
        _data_bit = _config & 0x01 ? 9 : 8;

		GPIO_InitTypeDef  GPIO_InitStructure = {0};
		USART_InitTypeDef USART_InitStructure = {0};

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		USART_InitStructure.USART_BaudRate = baud;
		//if(config == SERIAL_8N1)
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

		USART_Init(USART1, &USART_InitStructure);
		USART_Cmd(USART1, ENABLE);

    }

    inline size_t available(void)
    {
		
		if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
			return 0;
		else
			return 1;
    }

    inline uint8_t availableForWrite(void)
    {
		if(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
			return 0;
		return 1;
    }

    inline uint8_t read(void)
    {
		while(!available());
		uint8_t c = USART_ReceiveData(USART1);
        return c;
    }

	template <typename T>
    inline size_t write(T n)
    {
		while(!availableForWrite());
		USART_SendData(USART1, n);
		return 1;
    }

    inline size_t write(const uint8_t *buffer, size_t size)
    {
		for(size_t i = 0; i < size; i++)
			if(!write(buffer[i])) return i;
        return size;
    }

    inline void print(const char *s)
    {
        write((const uint8_t *)s, strlen(s));
    }
    inline void println(const char *s)
    {
        write((const uint8_t *)s, strlen(s));
        write((const uint8_t *)"\n", 1);
    }
	/*
    inline void printf(const char *format, ...)
		
    {
        char buf[64];
        va_list args;
        va_start(args, format);
        vsnprintf(buf, 64, format, args);
        va_end(args);
        write((const uint8_t *)buf, strlen(buf));
    }*/

    inline void flush()
    {
    }

    inline operator bool()
    {
        return true;//USBSerial_connected();
    }

private:
    uint32_t _baud;
    uint8_t _config;
    uint8_t _stop_bit;
    uint8_t _parity_bit;
    uint8_t _data_bit;
};
