#include "mbed.h"
#include <cstdint>
#include <cstring>

#define BUFFSIZE      64                            //Buffer size value

BufferedSerial pc(USBTX,USBRX, 115200);             //Serial connection for USB debugging output from F411
BufferedSerial xbee(PA_9,PA_10, 9600);              //Serial connection to Zigbee module

Thread readThread;                                  //Separate thread for reading simultaneously to sending

char msgBuff[BUFFSIZE]  = {0};                      //Buffer used to build and store payload
char xbeeBuff[BUFFSIZE] = {0};                      //Buffer used for building the complete Zigbee data frame

//Example complete message
char CompMsg[]          = {0x7E, 0x00, 0x10, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x48, 0x69, 0x42};
//Example portion of the pre-payload considered for the Checksum
char ChksmHeader[]      = {0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFE, 0x00, 0x00};

char buf[BUFFSIZE]      = {0};                      //Zigbee read buffer
char buffer[BUFFSIZE]   = {0};                      //USB output buffer
int USBLength;                                      //Length variable used for USB outputs

int counter;                                        //Counter variable used for dynamic message payload

void reader()
{
    while(1)                                        //Read thread while loop - Infinite
    {   
        if (uint32_t num = xbee.read(buf, sizeof(buf)))                                     //Blocking call IF statement to read from Zigbee module
        {
            USBLength = snprintf(buffer, BUFFSIZE, "\r\nThis is the message: %s", buf);     //Build a USB output message of what the received message says
            pc.write(buf, num);                                                             //Send the USB message
        }
    }
}

//Message is complete message and length is including all bytes, including checksum
char calculateChecksum(const char* message, int length) {
    int temp = 0;                                   //Temporary variable for calculating the sum of all bytes
    for(int i=3;i<length-1;i++)                     //Loop through pertinent bytes only
    {
        temp += message[i];                         //Add each pertinent byte's int value together
    }
    return (0xFF - temp) & 0xFF;                    //Subtract the total from 0xFF then keep only the last Byte (if more than one byte totalled)
}

void BuildMessage(char *xbeeMsg, char *msg, int len)
{
    char startByte[]        = {0x7E};               //Start delimiter - always the same
    char msgLen[]           = {0x00, 0x0E};         //Length of message - Header bytes without payload is 14 (0x0E)
    char type[]             = {0x10};               //Message type Transmit Request
    char frameID[]          = {0x01};               //Only sending one frame
    char destAddr[]         = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};         //Intended recipient - All zero is coordinator, all zero but last 2 bytes as F is broadcast, or the specific MAC address of recipient
    char destAdd2[]         = {0xFF, 0xFE};         //16-bit address of recipient if known - 0xFE means we don't know
    char broadcastRad[]     = {0x00};               //Default - Maximum number of network hops available
    char options[]          = {0x00};               //Default

    int finalLen = 17+len+1;                        //Calculate final length based on normal Transmit Request Header header plus payload plus checksum
    char checksum[1] = {0};                         //To store checksum for printing over USB            
    char _xbeeMsg[BUFFSIZE] = {0};                  //Buffer for complete message for transmission
    
    msgLen[1] += len;                               //Update length byte to add payload length

    char _xbeeMsg2[finalLen];                       //Prepare placeholder message array to return
    //Add all respective bytes to the placholder array as per their expected indexes
    _xbeeMsg2[0] = startByte[0];
    _xbeeMsg2[1] = msgLen[0];
    _xbeeMsg2[2] = msgLen[1];
    _xbeeMsg2[3] = type[0];
    _xbeeMsg2[4] = frameID[0];
    _xbeeMsg2[5] = destAddr[0];
    _xbeeMsg2[6] = destAddr[1];
    _xbeeMsg2[7] = destAddr[2];
    _xbeeMsg2[8] = destAddr[3];
    _xbeeMsg2[9] = destAddr[4];
    _xbeeMsg2[10] = destAddr[5];
    _xbeeMsg2[11] = destAddr[6];
    _xbeeMsg2[12] = destAddr[7];
    _xbeeMsg2[13] = destAdd2[0];
    _xbeeMsg2[14] = destAdd2[1];
    _xbeeMsg2[15] = broadcastRad[0];
    _xbeeMsg2[16] = options[0];

    //Loop through the message we are sending and add that to the payload portion of the placeholder
    for(int i=0;i<len;i++)                          
    {
        _xbeeMsg2[i+17] = msg[i];
    }

    checksum[0] = calculateChecksum(_xbeeMsg2, finalLen);           //Calculate checksum and store it in the holding char
    _xbeeMsg2[finalLen-1] = checksum[0];                            //Set the last message byte to the calculated checksum
    USBLength = snprintf(buffer, BUFFSIZE, "\r\nchecksum = %#x\r\n", checksum[0]);    //Prepare print out the Checksum to verify
    pc.write(buffer, USBLength);                                    //Write the checksum to USB UART

    xbee.write(_xbeeMsg2, finalLen);                                //Send complete message to module
    

    USBLength = snprintf(buffer, BUFFSIZE, "Hex: ");                //Build message to denote that the following will be a Hexadecimal representation of the full message
    pc.write(buffer, USBLength);                                    //Send message over USB
    for(int i=0;i<finalLen;i++)                                     //Loop through whole message and print out each character - this avoids zeros (0x00) from null terminating the message
    {
        USBLength = snprintf(buffer, BUFFSIZE, "%#x,", _xbeeMsg2[i]);
        pc.write(buffer, USBLength);
    }
}

// main() runs in its own thread in the OS
int main()
{
    pc.set_format(
        /* bits */ 8,
        /* parity */ BufferedSerial::None,
        /* stop bit */ 1
    );
    xbee.set_format(
        /* bits */ 8,
        /* parity */ BufferedSerial::None,
        /* stop bit */ 1
    );
    int n;
    n = snprintf(buffer, BUFFSIZE, "\r\n\r\nHello, I have started :)");
    pc.write(buffer, n);

    readThread.start(reader);

    while (true) {
        n = snprintf(msgBuff, BUFFSIZE, "counter %d", counter);
        BuildMessage(xbeeBuff, msgBuff, n);
        counter++;
        ThisThread::sleep_for(chrono::seconds(2));
    }
}

