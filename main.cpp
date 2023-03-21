#include "mbed.h"

#define BUFFERSIZE      64

BufferedSerial pc(USBTX,USBRX, 115200);
BufferedSerial xbee(PA_9,PA_10, 115200);

Thread readThread;

char buf[BUFFERSIZE] = {0};

void reader()
{
    while(1)
    {
        if (uint32_t num = xbee.read(buf, sizeof(buf))) {
            // Toggle the LED.
            //led = !led;

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

    while (true) {

    }
}

