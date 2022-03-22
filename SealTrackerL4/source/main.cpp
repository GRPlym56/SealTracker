#include "mbed.h"
#include "stm32l432xx.h"
#include "stm32l4xx_hal.h"
#include "LowPowerTicker.h"
#include "mbed_stats.h"


// main() runs in its own thread in the OS

//RTC_HandleTypeDef hrtc;

//EventQueue PrintQueue;
//Thread PrintThread;
//Thread TestThread;
DigitalOut LED(PB_3);

LowPowerTicker Tick;
void flashinterrupt(void);




int main()
{
    sleep_manager_can_deep_sleep();
    Tick.attach(flashinterrupt, 2s);
    

    while(1)
    {
        ThisThread::sleep_for(100s);
        //hal_deepsleep();
        //printf("Awoken\n\r");
        
    }
    /*
    while(1)
    {
        //HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
        //ThisThread::sleep_for(100s);
        //hal_deepsleep();
        //LED = !LED;
        //ThisThread::sleep_for(500ms);
    }
    */
    
}

void flashinterrupt()
{
    LED = !LED;
}
