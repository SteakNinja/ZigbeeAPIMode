#include "mbed.h"
#include <cstdint>

#define BUFFSIZE      64

BufferedSerial pc(USBTX,USBRX, 115200);
BufferedSerial xbee(PA_9,PA_10, 115200);

Thread readThread;

char startByte          = 0x7E;
uint16_t msgLen         = 13;
char type               = 0x10;
char frameID            = 0x01;
char destAddr[]         = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
char destAdd2           = 0xFE;
char broadcastRad       = 0x00;
char options            = 0x00;
char msgBuff[BUFFSIZE]  = {0};
char xbeeBuff[BUFFSIZE] = {0};

char CompMsg[]          = {0x7E, 0x00, 0x0F, 0x10, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0xA1, 0x58};
char msg2[]             = {0x10, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0xA1};

char buf[BUFFSIZE]      = {0};
char temp[BUFFSIZE]     = {0};
char buffer[BUFFSIZE]   = {0};
int length;
uint16_t checksum;

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

int BuildMessage(char *xbeeMsg, uint16_t *chcksum, char *msg, int len)
{
    int finalLen = 17+len;
    int temp;
    uint16_t temp2;
    int tempLen;
    char _xbeeMsg[BUFFSIZE] = {0};
    //length = snprintf(buffer, BUFFSIZE, "\r\nsize: %d", sizeof(message));
    //pc.write(buffer, length);
    char x = msgLen+len;
    int a = snprintf(_xbeeMsg, BUFFSIZE, "%c%c%c%c%c%s%c%c%c%s",startByte, 0x00, x, type, frameID, destAddr, destAdd2, broadcastRad, options, msg);
    
    //xbeeMsg = {0};

    for(int i = 3; i < a; i++)
    {
        temp += xbeeMsg[i];
    }
    temp2 = (0xFF-temp) & 0xFF;
    chcksum = &temp2;
    length = snprintf(buffer, BUFFSIZE, "\r\n%s", _xbeeMsg);
    pc.write(buffer, length);
    int b = snprintf(xbeeMsg, BUFFSIZE, "%s%c",_xbeeMsg,checksum);
    return finalLen;
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

    //int i = (0xFF - (0x10 + 0x01 + 0xFF + 0xFF + 0xFF + 0xFF + 0xFF + 0xFF + 0xFF + 0xFF + 0xFF + 0xFE + 0x00 + 0x00 + 0xA1));
    //int j = i & 0xFF; //Gets the least significant byte

    int n = snprintf(msgBuff, BUFFSIZE, "Hello");
    int m = BuildMessage(xbeeBuff, &checksum, msgBuff, n);
    //length = snprintf(buffer, BUFFSIZE, "\r\nmessage = %s", xbeeBuff);
    pc.write(xbeeBuff, m);

    while (true) {
        
        //xbee.write(msg, sizeof(msg));
        ThisThread::sleep_for(chrono::seconds(1));
    }
}

