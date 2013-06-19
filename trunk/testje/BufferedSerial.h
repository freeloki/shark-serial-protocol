/*
Arduino Buffered Serial
A library that helps establish buffered serial communication with a 
host application.
Copyright (C) 2010 Sigurður Örn Aðalgeirsson (siggi@media.mit.edu)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef BufferedSerial_h
#define BufferedSerial_h

#include "RingBuffer.h"

#ifndef byte
typedef unsigned char byte;
#endif
#ifndef word
typedef unsigned short word;
#endif

typedef void (*voidFuncPtr)(byte*, byte);
typedef int (*intFuncPtr)(byte, unsigned long);
typedef void (*voidFuncPtr2)(byte);


//!USART protocol constants
typedef enum{
    LEADING_FLAG=(byte)0xAA,		/*!<Start flag*/
    DLE_FLAG=(byte)0x10,			/*!<Bytestuffing flag*/
    CS_FLAG=(byte)0x30,             /*!<Checksum flag*/
    TRAILING_FLAG=(byte)0xFF        /*!<Trailing flag*/
}FLAGS;

typedef enum{
    WAITING,
    READING_PAYLOAD,
    READING_CHECKSUM
}STATE;


class BufferedSerial
{
public:
    BufferedSerial(unsigned int in_buf_size, unsigned int out_buf_size);
    void init(byte serial_port, unsigned long baud_rate, intFuncPtr serialPortInit);
    void setPacketHandler(voidFuncPtr);
    void handleIncomingByte(byte incoming);

	bool isBusySending();

	int sendSerialByte(byte b);
    int sendSerialPacket(RingBuffer* packet);
    int sendRawSerial(RingBuffer* packet);

    RingBuffer* outgoing_buffer;

private:
    void handlePacketDefault(byte* packet, byte length);
    void debugMessage(byte data);
    void debugMessage(const char* strData);

	voidFuncPtr handlePacketFunction;

	byte _serial_port;
    RingBuffer* incoming_buffer;
    byte* rxedPacket;
};

#endif

