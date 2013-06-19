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

#include <stdlib.h>
#include "../testje/BufferedSerial.h"
#include "../testje/RingBuffer.h"

bool flagCharacterInData(byte c);
word crc16_update(word crc, byte a);

BufferedSerial::BufferedSerial(unsigned int in_buf_size, unsigned int out_buf_size){
    incoming_buffer = new RingBuffer(in_buf_size);
    outgoing_buffer = new RingBuffer(out_buf_size);
    rxedPacket = (byte*)malloc(in_buf_size);
    handlePacketFunction = 0;
}

void BufferedSerial::init(byte serial_port, unsigned long baud_rate, intFuncPtr serialPortInit){
    _serial_port = serial_port;
    serialPortInit(serial_port, baud_rate);
}

bool BufferedSerial::isBusySending(){
    return ( outgoing_buffer->count() > 0 );
}

// Sends a single byte
int BufferedSerial::sendSerialByte(byte b){
    return outgoing_buffer->push(b);
}

// Sends a packet with a header and a checksum
int BufferedSerial::sendSerialPacket(RingBuffer* packet_buffer){
    // Copy buffer and calc checksum
    // http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html
    word checksum=0xFFFF;
    byte yChecksum, b;
    //Leading flag
    outgoing_buffer->push(LEADING_FLAG);
    while( packet_buffer->pop(b) > -1 )
    {
        /*Data:
         * if a databyte is equal to a flag character, then prepend
         * a DLE_FLAG to it.
         */
        if(flagCharacterInData(b))
        {
            outgoing_buffer->push(DLE_FLAG);
            checksum=crc16_update(checksum, DLE_FLAG);
            outgoing_buffer->push(~b);
        }
        else
        {
            outgoing_buffer->push(b);
        }
        checksum=crc16_update(checksum, b);
    }
    //Checksum flag (this will tell the RX-statemachine that CRC-calculation is complete)
    outgoing_buffer->push(CS_FLAG);
    //little endian checksum storage: send LSB of checksum first
    //also perform byte-stuffing on checksum (allows for simpler implementation)
    yChecksum=(byte)(checksum & 0xFF);
    if(flagCharacterInData(yChecksum))
    {
        outgoing_buffer->push(DLE_FLAG);
        outgoing_buffer->push(~yChecksum);
    }
    else{
        outgoing_buffer->push(yChecksum);
    }
    //Add MSB of checksum
    yChecksum=(byte)((checksum>>8) & 0xFF);
    if(flagCharacterInData(yChecksum))
    {
        outgoing_buffer->push(DLE_FLAG);
        outgoing_buffer->push(~yChecksum);
    }
    else{
        outgoing_buffer->push(yChecksum);
    }
    //Trailing flag
    outgoing_buffer->push(TRAILING_FLAG);
    return 0;
}

// Sends a raw packet with no header or checksum
int BufferedSerial::sendRawSerial(RingBuffer* packet_buffer){
    byte b;
    while( packet_buffer->pop(b) > -1 ){
        if(!outgoing_buffer->push(b))
        {
            return -1;
        }
    }
    return 0;
}

void BufferedSerial::setPacketHandler(voidFuncPtr packetHandlerFunction){
    handlePacketFunction = packetHandlerFunction;
}

/*! Function that reads the received characters from the
 *  circular buffer using a state machine.
 *  The databytes are read from the circular buffer, a check if performed
 *  if it is a flagbyte.  If it is, then the correct action is undertaken.
 *  If it is a databyte then that databyte is added to the framebuffer.
 *  A value is returned that tells us what this function has done with the
 *  data (if there was any).
 *  \param incoming incoming databyte.
 */
void BufferedSerial::handleIncomingByte(byte incoming){
    static bool dle_received=false;
    static word checksum;
    static STATE sState=WAITING;
    static byte yPacketByteCtr=0;
    byte a, b,c;
    word wChecksum;

    if(dle_received==false){
        switch(incoming){
        case LEADING_FLAG:
            incoming_buffer->reset();
            checksum=0xFFFF;
            sState=READING_PAYLOAD;
            break;
        case CS_FLAG:
            //All data bytes have come in
            yPacketByteCtr=0;
            while( incoming_buffer->pop(c) > -1){
                rxedPacket[yPacketByteCtr++]=c;
            }
            //Set state machine to other state, so that checksum bytes are stored to buffer,
            //but no CRC-calculation is done on these bytes.
            sState=READING_CHECKSUM;
            break;
        case TRAILING_FLAG:
            sState=WAITING;
            incoming_buffer->pop(a);
            incoming_buffer->pop(b);
            wChecksum=(((word)b)<<8)+a;
            if(wChecksum==checksum)
            {
                if( handlePacketFunction != 0 ){
                    handlePacketFunction(rxedPacket, yPacketByteCtr);
                }
                else{
                    handlePacketDefault(rxedPacket, yPacketByteCtr);
                }
            }
            break;
        case DLE_FLAG:
            dle_received=true;
            if(sState==READING_PAYLOAD){
                checksum=crc16_update(checksum,DLE_FLAG);
            }
            break;
        default:
            if(sState==READING_PAYLOAD){
                checksum=crc16_update(checksum,incoming);
            }
            incoming_buffer->push(incoming);
        }//switch
    }
    else
    {
        incoming=~incoming;
        if(sState==READING_PAYLOAD){
            checksum=crc16_update(checksum,incoming);
        }
        incoming_buffer->push(incoming);
        dle_received=false;
    }
}

void BufferedSerial::handlePacketDefault(byte* packet, byte length){
    // We could do something here like send the data to the host again for debug
    // Or just do nothing
    return;
}

/*!Check if the character is a flag character.
 *\param c The character
 *\return TRUE when c is a flag character else FALSE.
 */
bool flagCharacterInData(byte c){
    if(c==LEADING_FLAG||
            c==TRAILING_FLAG||
            c==DLE_FLAG||
            c==CS_FLAG)
    {
        return true;
    }
    else return false;
}//flagCharacterInData

word crc16_update(word crc, byte a)
{
    int i;

    crc ^= a;
    for (i = 0; i < 8; ++i)
    {
        if (crc & 1)
            crc = (crc >> 1) ^ 0xA001;
        else
            crc = (crc >> 1);
    }

    return crc;
}
