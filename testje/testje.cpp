//QtCreator info:
//  http://www.jayway.com/2011/09/22/using-qtcreator-for-arduino-development/
//  http://sourceforge.net/projects/qtarduino/
// Do not remove the include below
#include "testje.h"

const int SEND_BUFFER_SIZE=101;
const byte SERIAL_PORT=0;

BufferedSerial serial = BufferedSerial(150,150);
RingBuffer send_buffer= RingBuffer(SEND_BUFFER_SIZE);
SerialCommand sCmd;

void handlePacket(byte* packet, byte length){
    sCmd.readSerial(packet, length);
}

//Process the request for the number of bytes that can be pushed in the sound buffer
//and respond.
void respondData(byte* packet, byte length){
    send_buffer.push(1);
    for(int i=0;i<SEND_BUFFER_SIZE-1;i++){
        send_buffer.push(2);
    }
    serial.sendSerialPacket(&send_buffer);
}

int openComport(byte serial_port, unsigned long baud_rate){
    if( serial_port == 0)
        Serial.begin(baud_rate);
}

void receiveBuffer(){
    // If we have received stuff
    if( SERIAL_PORT == 0 ){
        while( Serial.available() != 0 ) {
            byte incoming = Serial.read();
            serial.handleIncomingByte(incoming);
        }
    }
}

void transmitBuffer(){
    byte b;
    byte buf[150];
    // If we should be sending stuff
    int i=0;
    while( serial.outgoing_buffer->pop(b) > -1 )
    {
        Serial.write(b);
        //buf[i++]=b;
    }
    //Serial.write(buf, i);
}

unsigned long lstartTime;
void loop() {
    receiveBuffer();
    transmitBuffer();
//    if(millis()-lstartTime>1000){
//        respondData(0,0);
//        lstartTime=millis();
//    }
}

void setup()
{
    // initialize the serial communication:
    serial.init(SERIAL_PORT, 115200, openComport);
    serial.setPacketHandler(handlePacket);
    sCmd.addCommand(1, respondData);

    lstartTime=millis();
}

