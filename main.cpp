#include "mbed.h"
#include <cstdint>
#include <cstring>

#define BUFFSIZE      64

BufferedSerial pc(USBTX,USBRX, 115200);
BufferedSerial xbee(PA_9,PA_10, 115200);

Thread readThread;


char msgBuff[BUFFSIZE]  = {0};
char xbeeBuff[BUFFSIZE] = {0};

char CompMsg[]          = {0x7E, 0x00, 0x0F, 0x10, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0xA1, 0x58};
char ChksmPreamble[]    = {0x10, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x00, 0x00};

char buf[BUFFSIZE]      = {0};
char buffer[BUFFSIZE]   = {0};
int length;
int checksum;

int counter;

void reader()
{
    while(1)
    {
        if (uint32_t num = xbee.read(buf, sizeof(buf))) {
            
            length = snprintf(buffer, BUFFSIZE, "\r\nThis is the message: %s", buf);
            pc.write(buf, num);
        }
    }
}

void BuildMessage(char *xbeeMsg, char *msg, int len)
{
    char startByte[]        = {0x7E};
    char msgLen[]           = {0x00, 0x0E};
    char type[]             = {0x10};
    char frameID[]          = {0x01};
    char destAddr[]         = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    char destAdd2[]         = {0xFF, 0xFE};
    char broadcastRad[]     = {0x00};
    char options[]          = {0x00};

    int templen = 0;
    int finalLen = 17+len;
    int temp = 0;
    char temp2[1] = {0};
    char _xbeeMsg[BUFFSIZE] = {0};
    
    //Calculate checksum
    for(int i=0;i<msgLen[1];i++)
    {
        temp += ChksmPreamble[i];
    }
    for(int i=0;i<len;i++)
    {
        temp += msg[i];
    }
    msgLen[1] += len;
    temp2[0] = (0xFF-temp) & 0xFF;

    finalLen = msgLen[1]+4;                                                     //Calculate full message length

    length = snprintf(buffer, BUFFSIZE, "\r\nchecksum = %#x\r\n", temp2[0]);    //Prepare print out the Checksum to verify
    pc.write(buffer, length);                                                   //Write the checksum to USB UART

    //Prepare full Xbee message frame
    char _xbeeMsg2[finalLen];
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
    for(int i=0;i<len;i++)
    {
        _xbeeMsg2[i+17] = msg[i];
    }
    _xbeeMsg2[finalLen-1] = temp2[0];

    xbee.write(_xbeeMsg2, finalLen);                                //Send to xbee
    

    length = snprintf(buffer, BUFFSIZE, "Hex: ");
    pc.write(buffer, length);
    for(int i=0;i<finalLen;i++)
    {
        length = snprintf(buffer, BUFFSIZE, "%#x,", _xbeeMsg2[i]);
        pc.write(buffer, length);
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
    readThread.start(reader);
    int n;
    n = snprintf(msgBuff, BUFFSIZE, "Hello, I have started :)");
    BuildMessage(xbeeBuff, msgBuff, n);

    while (true) {
        n = snprintf(msgBuff, BUFFSIZE, "counter %d", counter);
        BuildMessage(xbeeBuff, msgBuff, n);
        counter++;
        ThisThread::sleep_for(chrono::seconds(2));
    }
}

