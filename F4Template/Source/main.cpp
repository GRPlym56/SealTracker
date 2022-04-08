#include "mbed.h"
#include "nRF24L01P.h"
#include "CommsWrapper.hpp"
#include "Azure.hpp"
#include "Config.hpp"
#include "SDWrapper.hpp"

 
/*
TODO
add buffer for samples
Commswrapper puts samples on the buffer
SD takes them off
add peek function in case something else needs them later

*/


//Network Azure;
//NetworkSampleConsumer AzureConsumer(&Azure);

CommsWrapper RFModule(RFPINS);
SDCARD microSD(SDpins);

EventQueue PrintQueue;
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
        sleep();
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