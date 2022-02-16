#include "mbed.h"

// main() runs in its own thread in the OS

EventQueue PrintQueue;
Thread PrintThread;
Thread TestThread;
DigitalOut LED(PB_3);

void flash(void);
void printer(void);

int main()
{
    PrintThread.start(printer);
    TestThread.start(flash);

}

void flash()
{
    while(1){
        LED = !LED;
        ThisThread::sleep_for(1s);
    }
}

void printer()
{
    PrintQueue.dispatch_forever();
} 