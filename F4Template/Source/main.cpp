#include "mbed.h"
#include "nRF24L01P.h"
#include "CommsWrapper.hpp"
#include "Azure.hpp"
#include "Config.hpp"
#include "SDWrapper.hpp"

 



//Network Azure;
//NetworkSampleConsumer AzureConsumer(&Azure);

CommsWrapper RFModule(RFPINS);
SDCARD microSD(SDpins);

EventQueue PrintQueue;
Thread RFThread;

 
void ReceiveData(void);

int main() {

    RFModule.InitReceiveNode();
    RFThread.start(ReceiveData);
    
    

    while (1)
    {
        sleep();
    }

}

void ReceiveData()
{
    RFModule.ReceiveData(); //main receive loop
}