#include "mbed.h"
#include "nRF24L01P.h"
#include "CommsWrapper.hpp"
#include "Azure.hpp"
#include "Config.hpp"
#include "SDWrapper.hpp"
#include "CBUFF.hpp"

 
/*
TODO
add buffer for samples
Commswrapper puts samples on the buffer
SD takes them off
add peek function in case something else needs them later

*/


//Network Azure;
//NetworkSampleConsumer AzureConsumer(&Azure);

CircBuff SampleBuffer(512, "MainBuff");

EventQueue PrintQueue;
CommsWrapper RFModule(RFPINS, PB_7);
SDCARD microSD(SDpins);


Thread RFThread;
Thread PrintThread;


 
void ReceiveData(void);
void Printer();

int main() {


    PrintThread.start(Printer);
    RFModule.InitReceiveNode();
    RFThread.start(ReceiveData);
    
    microSD.Test();
    

    while (1)
    {
        ThisThread::sleep_for(60s);
        
        microSD.flush();
    }

}

void ReceiveData()
{
    RFModule.ReceiveData(); //main receive loop
}

void Printer()
{
    PrintQueue.dispatch_forever();
}

