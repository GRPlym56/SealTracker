#ifndef __COMMSWRAPPER__
#define __COMMSWRAPPER__

#include "mbed.h"
#include "nRF24L01P.h"
#include "Config.hpp"
#include "CBUFF.hpp"
#include <iostream>
#include <string>

extern EventQueue PrintQueue;

#define TRANSFER_SIZE 32 //max message size 


class CommsWrapper
{
    public:

    CommsWrapper(NRFPINS pins);
    void ReceiveData();
    void InitSendNode();
    void InitReceiveNode();
    void Sendmsg(char msg[]);
    void SendmsgNoPwrCntrl(char msg[]);

    void RequestTime();

    void On();
    void Off();
  

    private:

    nRF24L01P Comms;   
    char txData[TRANSFER_SIZE], rxData[TRANSFER_SIZE];
    int txDataCnt = 0;
    int rxDataCnt = 0;

  

    
    
    
    
    
    


};


#endif