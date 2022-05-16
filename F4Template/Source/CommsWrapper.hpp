#ifndef __COMMSWRAPPER__
#define __COMMSWRAPPER__

#include "mbed.h"
#include "nRF24L01P.h"
#include "Config.hpp"
#include "CBUFF.hpp"
#include <iostream>
#include <string>

#define TRANSFER_SIZE 32
#define DATARATE NRF24L01P_DATARATE_2_MBPS

extern EventQueue PrintQueue;
extern bool skipsetup;

class CommsWrapper
{
    public:

    CommsWrapper(NRFPINS pins, DigitalOut CommsLED, CircBuff* SD, CircBuff* Net, ButtonConfig_t BPins);
    void Sendmsg(char msg[]);
    void ReceiveData();
    void InitSendNode();
    void InitReceiveNode();
    void Decode();

    void WaitForRequest();

    void SetAzureThreadID(osThreadId_t threadID);

    private:

    nRF24L01P Comms;   
    char txData[32], rxData[32];
    int txDataCnt = 0;
    int rxDataCnt = 0;

    DigitalOut LED;

    CircBuff* sdbuff;
    CircBuff* netbuff;
    
    osThreadId_t AzureThread;

   DigitalIn SW1;
    
    
    


};


#endif