#include "Azure.hpp"
#include <chrono>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <string>
#include <string.h>
#include <fstream>



Azure::Azure(CircBuff* NetBuff): NetBuffer(NetBuff) 
{
   /* 
    Connect();
    
    setTime();
    Sendrate = 10s;
    */

}

bool message_received = false;

bool Azure::Connect()
{
    PrintQueue.call(printf, "Connecting to the network\n\r");
    printf("Connecting to network\r\n");

    _defaultSystemNetwork = NetworkInterface::get_default_instance();
    if (_defaultSystemNetwork == nullptr) {
        PrintQueue.call(printf, "No network interface found\n\r");
        return false;
    }

    int ret = _defaultSystemNetwork->connect();
    if (ret != 0) {
        PrintQueue.call(printf, "Connection error: %d\n\r", ret);
        return false;
    }
    PrintQueue.call(printf, "Connection success, MAC: %s", _defaultSystemNetwork->get_mac_address());
    return true;
}


bool Azure::setTime()
{
    PrintQueue.call(printf, "Getting time from the NTP server\n\r");
    printf("Getting time from the NTP server\n\r");

    NTPClient ntp(_defaultSystemNetwork);
    ntp.set_server("time.google.com", 123);
    timestamp = ntp.get_timestamp();
    tm *ltm = localtime(&timestamp);
    if (timestamp < 0) {
        PrintQueue.call(printf, "Failed to update the current time, error: %ud\n\r", timestamp);
        return false;
    }
    PrintQueue.call(printf, "Time: %s\n\r", ctime(&timestamp));
    set_time(timestamp);
    return true;
}


void Azure::SendData() {
    //bool trace_on = MBED_CONF_APP_IOTHUB_CLIENT_TRACE;
    bool trace_on = 0;
    tickcounter_ms_t interval = 100;
    IOTHUB_CLIENT_RESULT res;

    LogInfo("Initializing IoT Hub client");
    IoTHub_Init();



    IOTHUB_DEVICE_CLIENT_HANDLE client_handle = IoTHubDeviceClient_CreateFromConnectionString(
        azure_cloud::credentials::iothub_connection_string,
        MQTT_Protocol
    );
    if (client_handle == nullptr) {
        LogError("Failed to create IoT Hub client handle");
        goto cleanup;
    }

    // Enable SDK tracing
    res = IoTHubDeviceClient_SetOption(client_handle, OPTION_LOG_TRACE, &trace_on);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to enable IoT Hub client tracing, error: %d", res);
        goto cleanup;
    }

    // Enable static CA Certificates defined in the SDK
    res = IoTHubDeviceClient_SetOption(client_handle, OPTION_TRUSTED_CERT, certificates);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to set trusted certificates, error: %d", res);
        goto cleanup;
    }

    // Process communication every 100ms
    res = IoTHubDeviceClient_SetOption(client_handle, OPTION_DO_WORK_FREQUENCY_IN_MS, &interval);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to set communication process frequency, error: %d", res);
        goto cleanup;
    }

    // set incoming message callback
    res = IoTHubDeviceClient_SetMessageCallback(client_handle, on_message_received, nullptr);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to set message callback, error: %d", res);
        goto cleanup;
    }

    // Set incoming command callback
    res = IoTHubDeviceClient_SetDeviceMethodCallback(client_handle, on_method_callback, nullptr);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to set method callback, error: %d", res);
        goto cleanup;
    }

    // Set connection/disconnection callback
    res = IoTHubDeviceClient_SetConnectionStatusCallback(client_handle, on_connection_status, nullptr);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to set connection status callback, error: %d", res);
        goto cleanup;
    }

   

    // Send ten message to the cloud (one per second)
    // or until we receive a message from the cloud
    IOTHUB_MESSAGE_HANDLE message_handle;
    char message[80];

    /*
        MAKE THIS WAIT ON A SIGNAL INSTEAD OF JUST POLLING AS THAT PROBABLY ISNT A VERY GOOD WAY OF DOING IT
    */

    while(1){

        if (message_received) {
            // If we have received a message from the cloud, don't send more messeges
            break;
        }
        ThisThread::sleep_for(250ms); //does not need to rapidly poll, NRF data in is higher priority
        ThisThread::flags_wait_all(AzureFlag, false); //wait on a flag, dont clear until we are certain the buffer is empty

        if(NetBuffer->EmptyCheck())
        {

            PrintQueue.call(printf, "Netbuff empty, nothing to send\n");
            ThisThread::flags_clear(AzureFlag); //clear flag
            
        }else 
        {
                //Send data in this format:
            /*
                {
                    "Pressure" : 1000.5,
                    "Temperature" : 21.3
                    "Time" :  oct,17,21:09
                }

            */
            sealsample_t outputData = NetBuffer->Get(); //get samples from buffer 
            
            sprintf(message, "{ \"Pressure\" : %s, \"Temperature\" : %s, \"Time\" : %s }", outputData.pressure.c_str(), outputData.temperature.c_str(), outputData.time.c_str());
            LogInfo("Sending: \"%s\"", message);

            message_handle = IoTHubMessage_CreateFromString(message);
            if (message_handle == nullptr) {
                LogError("Failed to create message");
                goto cleanup;
            }

            res = IoTHubDeviceClient_SendEventAsync(client_handle, message_handle, on_message_sent, nullptr);
            IoTHubMessage_Destroy(message_handle); // message already copied into the SDK

            if (res != IOTHUB_CLIENT_OK) {
                LogError("Failed to send message event, error: %d", res);
                goto cleanup;
            }

            
        }
        
        
    }

    // If the user didn't manage to send a cloud-to-device message earlier,
    // let's wait until we receive one
    while (!message_received) {
        // Continue to receive messages in the communication thread
        // which is internally created and maintained by the Azure SDK.
        sleep();
    }

cleanup:
    IoTHubDeviceClient_Destroy(client_handle);
    IoTHub_Deinit();
}

/*
    -----------NON OO BELOW-----------
*/



void on_connection_status(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context)
{
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED) {
        LogInfo("Connected to IoT Hub");
    } else {
        LogError("Connection failed, reason: %s", MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONNECTION_STATUS_REASON, reason));
    }
}

// **************************************
// * MESSAGE HANDLER (no response sent) *
// **************************************

IOTHUBMESSAGE_DISPOSITION_RESULT on_message_received(IOTHUB_MESSAGE_HANDLE message, void* user_context)
{
    LogInfo("Message received from IoT Hub");

    const unsigned char *data_ptr;
    size_t len;
    if (IoTHubMessage_GetByteArray(message, &data_ptr, &len) != IOTHUB_MESSAGE_OK) {
        LogError("Failed to extract message data, please try again on IoT Hub");
        return IOTHUBMESSAGE_ABANDONED;
    }

    message_received = true;
    LogInfo("Message body: %.*s", len, data_ptr);

    if (strncmp("true", (const char*)data_ptr, len) == 0) {
        
    } else {
        
    }

    return IOTHUBMESSAGE_ACCEPTED;
}

void on_message_sent(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    if (result == IOTHUB_CLIENT_CONFIRMATION_OK) {
        LogInfo("Message sent successfully");
    } else {
        LogInfo("Failed to send message, error: %s",
            MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
    }
}


// ****************************************************
// * COMMAND HANDLER (sends a response back to Azure) *
// ****************************************************


int on_method_callback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* response_size, void* userContextCallback)
{
    const char* device_id = (const char*)userContextCallback;

    printf("\r\nDevice Method called for device %s\r\n", device_id);
    printf("Device Method name:    %s\r\n", method_name);
    printf("Device Method payload: %.*s\r\n", (int)size, (const char*)payload);

    if ( strncmp("true", (const char*)payload, size) == 0 ) {
        printf("LED ON\n");
        
    } else {
        printf("LED OFF\n");
        
    }

    int status = 200;
    //char RESPONSE_STRING[] = "{ \"Response\": \"This is the response from the device\" }";
    char RESPONSE_STRING[64];
    //sprintf(RESPONSE_STRING, "{ \"Response\" : %d }", blueButton.read());

    printf("\r\nResponse status: %d\r\n", status);
    printf("Response payload: %s\r\n\r\n", RESPONSE_STRING);

    int rlen = strlen(RESPONSE_STRING);
    *response_size = rlen;
    if ((*response = (unsigned char*)malloc(rlen)) == NULL) {
        status = -1;
    }
    else {
        memcpy(*response, RESPONSE_STRING, *response_size);
    }
    return status;
}