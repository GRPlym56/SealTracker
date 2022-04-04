#include "mbed.h"
#include "nRF24L01P.h"
#include "CommsWrapper.hpp"
//#include "network.hpp"
#include "Config.hpp"

 

#define TRANSFER_SIZE   32

//Network Azure;
//NetworkSampleConsumer AzureConsumer(&Azure);

CommsWrapper RFModule(RFPINS);


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