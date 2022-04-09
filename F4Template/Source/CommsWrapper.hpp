#ifndef __COMMSWRAPPER__
#define __COMMSWRAPPER__

#include "mbed.h"
#include "nRF24L01P.h"
#include "Config.hpp"
#include "CBUFF.hpp"
#include <iostream>
#include <string>

extern EventQueue PrintQueue;
extern CircBuff SampleBuffer;


class CommsWrapper
{
    public:

    CommsWrapper(NRFPINS pins, DigitalOut CommsLED);
    void ReceiveData();
    void InitSendNode();
    void InitReceiveNode();

    private:

    nRF24L01P Comms;   
    char txData[32], rxData[32];
    int txDataCnt = 0;
    int rxDataCnt = 0;

    DigitalOut LED;
    sealsample_t rxDataFormatted;

    
    void Decode();
    
    
    


};


#endif