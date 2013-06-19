#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "../testje/RingBuffer.h"
#include "../testje/BufferedSerial.h"
#include "SerialCommand.h"
#include "rs232.h"

const int TTYACM0=24;
const int SEND_BUFFER_SIZE=101;
const int RUNTIME_MS=10000;

BufferedSerial serial = BufferedSerial(256, 256);
RingBuffer send_buffer= RingBuffer(SEND_BUFFER_SIZE);
SerialCommand sCmd;
struct timeval start, startLoop;
unsigned int uiRxPacketCtr, uiTxPacketCtr;

void handlePacket(byte* packet, byte length){
    sCmd.readSerial(packet, length);
}

void respondData(byte* packet, byte length){
    uiRxPacketCtr++;
}

int openComport(byte serial_port, unsigned long baud_rate){
    if(RS232_OpenComport(serial_port, baud_rate))
    {
        printf("Can not open comport\n");
    }
}

void receiveBuffer(){
    byte buf[4096];
    int n = RS232_PollComport(TTYACM0, buf, 4095);
    if(n > 0)
    {
        for(int i=0; i < n; i++)
        {
            serial.handleIncomingByte(buf[i]);
        }
    }
}

void transmitBuffer(){
    byte b;
    byte buf[4096];
    int i=0;
    while( serial.outgoing_buffer->pop(b) > -1 )
    {
        buf[i++]=b;
    }
    //Sending data byte by byte is SLOOOOOW. Don't do that if speed is a concern.
    //Save all the data you want to send (e.g. a packet) in a buffer.  Then send
    //the buffer at once to the serial port.
    RS232_SendBuf(TTYACM0, buf,i);
}

void loop() {
    receiveBuffer();
    transmitBuffer();

    struct timeval end;
    long mtime, seconds, useconds;

    gettimeofday(&end, NULL);
    seconds  = end.tv_sec  - startLoop.tv_sec;
    useconds = end.tv_usec - startLoop.tv_usec;
    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

    if( (!serial.isBusySending()) && mtime>10)
    {
        gettimeofday(&startLoop, NULL);

        send_buffer.push(1);
        for(int i=0;i<SEND_BUFFER_SIZE-1;i++){
            send_buffer.push(2);
        }
        serial.sendSerialPacket( &send_buffer );
        uiTxPacketCtr++;
    }
    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
    if(mtime > RUNTIME_MS){
        printf("%d packets received\n%d packets sent\nRX data rate = %dbytes/s\nTX data rate = %dbytes/s\n",
               uiRxPacketCtr, uiTxPacketCtr,
               uiRxPacketCtr*(SEND_BUFFER_SIZE-1)*1000/RUNTIME_MS,
               uiTxPacketCtr*(SEND_BUFFER_SIZE-1)*1000/RUNTIME_MS);
        exit(0);
    }
}

void setup()
{
    // initialize the serial communication:
    serial.init(TTYACM0, 115200, openComport);
    serial.setPacketHandler(handlePacket);
    sCmd.addCommand(1, respondData);
    gettimeofday(&start, NULL);
    gettimeofday(&startLoop, NULL);
    uiRxPacketCtr=0;
    uiTxPacketCtr=0;
}

int main()
{
    setup();
    while(1){
        loop();
    }
}



