#include "mbed.h"
#include "stm32l432xx.h"
#include "stm32l4xx_hal.h"
#include "LowPowerTicker.h"
#include "mbed_stats.h"
#include "sx1280-hal.h"
#include "radio.h"
#include "sx1280.h"

// main() runs in its own thread in the OS

//RTC_HandleTypeDef hrtc;

//EventQueue PrintQueue;
//Thread PrintThread;
//Thread TestThread;
DigitalOut LED(PB_3);

LowPowerTicker Tick;
void flashinterrupt(void);

RadioCallbacks_t callbacks =
{
    &OnTxDone,        // txDone
    &OnRxDone,        // rxDone
    NULL,             // syncWordDone
    NULL,             // headerDone
    &OnTxTimeout,     // txTimeout
    &OnRxTimeout,     // rxTimeout
    &OnRxError,       // rxError
    &OnRangingDone,   // rangingDone
    NULL,             // cadDone
};

SX1280Hal RF(D11, D12, PB_3, A3, D6, D7, D8, D2, D9, &callbacks);



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

void OnTxDone( void )
{
    AppState = APP_TX;
}
 
void OnRxDone( void )
{
    AppState = APP_RX;
}
 
void OnTxTimeout( void )
{
    AppState = APP_TX_TIMEOUT;
    printf( "<>>>>>>>>TXE\r\n" );
}
 
void OnRxTimeout( void )
{
    AppState = APP_RX_TIMEOUT;
}
 
void OnRxError( IrqErrorCode_t errorCode )
{
    AppState = APP_RX_ERROR;
    printf( "RXE<>>>>>>>>\r\n" );
}
 
void OnRangingDone( IrqRangingCode_t val )
{
    if( val == IRQ_RANGING_MASTER_VALID_CODE || val == IRQ_RANGING_SLAVE_VALID_CODE )
    {
        AppState = APP_RANGING_DONE;
    }
    else if( val == IRQ_RANGING_MASTER_ERROR_CODE || val == IRQ_RANGING_SLAVE_ERROR_CODE )
    {
        AppState = APP_RANGING_TIMEOUT;
    }
    else
    {
        AppState = APP_RANGING_TIMEOUT;
    }
}
 
void OnCadDone( bool channelActivityDetected )
{
}


