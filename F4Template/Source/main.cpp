#include "mbed.h"
#include "nRF24L01P.h"
#include "CommsWrapper.hpp"
#include "Azure.hpp"
#include "Config.hpp"
#include "SDWrapper.hpp"
#include "CBUFF.hpp"


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


    PrintThread.start(Printer);
    RFModule.InitReceiveNode();
    RFThread.start(ReceiveData);
    AzureThread.start(Networking);

    microSD.Test();
    

    while (1)
    {
        ThisThread::sleep_for(60s);
        
        
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