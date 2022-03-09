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

/*
void Error_Handler(void);

void SystemClock_Config(void);
static void MX_RTC_Init(void);
*/

int main()
{

    //HAL_Init();
    //SystemClock_Config();
    //MX_RTC_Init();
    
    Tick.attach(flashinterrupt, 2s);
    
    
    //hal_deepsleep();


    while(1)
    {
        //HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
        hal_deepsleep();
        //LED = !LED;
        //ThisThread::sleep_for(500ms);
    }
    
}

void flashinterrupt()
{
    LED = !LED;
}




