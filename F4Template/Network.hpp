#ifndef _NETWORK_H__
#define _NETWORK_H__
#include "mbed.h"
//#include "uop_msb.h"
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

//#include "sampleReceiver.hpp"




extern NetworkInterface *_defaultSystemNetwork;
extern EventQueue PrintQueue;

class Network
{ 
    public:
        Network();
        bool connect(); //establish network connection
        bool setTime(); //set current time
        time_t getTime(); //get current time
      
        
        void setThresholds(unsigned short lim, int EnviVar, int Thresh); //set thresholds 
        void SetSendRate(std::chrono::milliseconds rate);
        std::chrono::milliseconds GetSendRate();
        void SendData(); //send data and set up message callbacks
       
        

    private:
        
        Mutex Threshlock; //Lock for setting thresholds safely
        Mutex ParamLock;
        int DisplayParam = 0;
        std::chrono::milliseconds sendrate;
        time_t timestamp;
        //static Thresholds_t Lim;
       
        
        //       bool message_received = false;
};

//non OO jank >:(

int on_method_callback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* response_size, void* userContextCallback);
void on_message_sent(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback);
void on_connection_status(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context);
IOTHUBMESSAGE_DISPOSITION_RESULT on_message_received(IOTHUB_MESSAGE_HANDLE message, void* user_context);
enum {LIGHT = 0, TEMP, PRESS};

#endif