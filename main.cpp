#include "mbed.h"

#define BUFFERSIZE      64

BufferedSerial pc(USBTX,USBRX, 115200);
BufferedSerial xbee(PA_9,PA_10, 115200);

Thread readThread;

char startByte = 0x7E;


char buf[BUFFERSIZE] = {0};
char temp[BUFFERSIZE] = {0};
char buffer[BUFFERSIZE] = {0};
int length;

void reader()
{
    while(1)
    {
        if (uint32_t num = xbee.read(buf, sizeof(buf))) {
            // Toggle the LED.
            //led = !led;
            //length = snprintf(temp,BUFFERSIZE,"%s",buf);
            // Echo the input back to the terminal.
            pc.write(buf, num);
        }
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

    int i = (0xFF - (0x10 + 0x01 + 0xFF + 0xFF + 0xFF + 0xFF + 0xFF + 0xFF + 0xFF + 0xFF + 0xFF + 0xFE + 0x00 + 0x00 + 0x41));
        length = snprintf(buffer, BUFFERSIZE, "%x", i);
        pc.write(buffer, length);

    while (true) {
        ThisThread::sleep_for(chrono::seconds(10));
    }
}

