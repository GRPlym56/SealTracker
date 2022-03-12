/*
ELEC-351 Group E
Author: Guy Ringshaw
December-January 2022
*/

#ifndef _NETWORK_H__
#define _NETWORK_H__
#include "mbed.h"
#include "uop_msb.h"
#include "NTPClient.h"


#include "rtos/ThisThread.h"
#include "certs.h"
#include "iothub.h"
#include "iothub_client_options.h"
#include "iothub_device_client.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/xlogging.h"

#include "iothubtransportmqtt.h"
#include "azure_cloud_credentials.h"
#include <cstring>
#include <string.h>
#include "Config.hpp"
#include "Matrix.hpp"
#include "sampleReceiver.hpp"




extern NetworkInterface *_defaultSystemNetwork;


class Network: public ConsumerContext{
    public:
        Network();
        bool connect(); //establish network connection
        bool setTime(); //set current time
        time_t getTime(); //get current time
        Thresholds_t getThresholds() override; //overrided function for getting thresholds
        int getDisplayParam() override; //overrided function for getting current samples
        void setDisplayParam(int Param); //set parameter to display on the LED matrix
        void setThresholds(unsigned short lim, int EnviVar, int Thresh); //set thresholds 
        void SetSendRate(milliseconds rate);
        milliseconds GetSendRate();
        void SendData(); //send data and set up message callbacks
        Circular_Buff <sample_t> netBuff; //network sample buffer
        

    private:
        
        void getThreshLock(void) override; //overrided function, mutex lock for setting thresholds since they are used throughout the program
        void releaseThreshLock(void) override; //overrided function, release lock
        void getParamLock(void) override;
        void releaseParamLock(void) override;
        Mutex Threshlock; //Lock for setting thresholds safely
        Mutex ParamLock;
        int DisplayParam = 0;
        milliseconds sendrate;
        time_t timestamp;
        //static Thresholds_t Lim;
        Thresholds_t limits{
               {20,60},        //Light
               {15,30},        //Temp
               {950, 1150}     //Press
        };
        
        //       bool message_received = false;
};

//non OO jank >:(

int on_method_callback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* response_size, void* userContextCallback);
void on_message_sent(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback);
void on_connection_status(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context);
IOTHUBMESSAGE_DISPOSITION_RESULT on_message_received(IOTHUB_MESSAGE_HANDLE message, void* user_context);
enum {LIGHT = 0, TEMP, PRESS};

#endif