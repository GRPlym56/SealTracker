#include "mbed.h"
#include "nRF24L01P.h"
#include "CommsWrapper.hpp"
#include "Azure.hpp"
#include "Config.hpp"
#include "SDWrapper.hpp"
#include "CBUFF.hpp"
#include <ratio>




EventQueue PrintQueue;
CircBuff SDBuffer(256, "SDBuff");
CircBuff NetBuffer(256, "NetBuff");

SDCARD microSD(SDpins, &SDBuffer);
Azure Net(&NetBuffer);
CommsWrapper RFModule(RFPINS, PB_7, &SDBuffer, &NetBuffer);

Thread RFThread;
Thread PrintThread;
Thread AzureThread;

void ReceiveData();
void Printer();
void Networking();


int main() {

    //startup section, attempt to send time to L432kc
    PrintThread.start(Printer);

    RFModule.WaitForRequest(); //wait for the L432 to request time

    RFModule.InitReceiveNode();
    RFThread.start(ReceiveData);
    

    RFModule.SetAzureThreadID(AzureThread.get_id()); //give azure thread ID to the commswrapper

    microSD.Test(); //write a simple text file 
    

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

void Networking()
{
    Net.SendData();
}