#include "mbed.h"
#include "nRF24L01P.h"
#include "CommsWrapper.hpp"
#include "network.hpp"
#include "Config.hpp"
#include "sampleReceiver.hpp"
#include "NetworkSampleConsumer.hpp"
 

#define TRANSFER_SIZE   32

Network Azure;
NetworkSampleConsumer AzureConsumer(&Azure);

CommsWrapper RFModule(RFPINS);
 
DigitalOut myled1(LED1);
DigitalOut myled2(LED2);

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