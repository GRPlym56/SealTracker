#include "mbed.h"
#include "stm32l432xx.h"
#include "stm32l4xx_hal.h"
#include "LowPowerTicker.h"

// main() runs in its own thread in the OS

//RTC_HandleTypeDef hrtc;

//EventQueue PrintQueue;
//Thread PrintThread;
//Thread TestThread;
DigitalOut LED(PB_3);

LowPowerTicker Tick;
void flashinterrupt(void);
Timer tm1;
int SampleCounter;

int main()
{
    LED = 0;
    SampleCounter = 0;
    //tm1.reset();
    
    Tick.attach(flashinterrupt, 1s); //this will continuously wake the system from deep sleep to do stuff
    
    //ThisThread::sleep_for(10s);

    while(1)
    {
        hal_deepsleep();
        
       

        /*
        LED = !LED;
        tm1.start();
        //HAL_Delay(5000);
        ThisThread::sleep_for(5s);
        printf("Delay: %llu \n\r", tm1.elapsed_time());
        tm1.reset();
        //ThisThread::sleep_for(1000ms);
        */

        /*
        LED = 1;
        HAL_Delay(1000);
        LED = 0;
        */

    }
    
}

void flashinterrupt()
{
    LED = !LED;
    

    
}




