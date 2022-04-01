#include "network.hpp"
//#include "SD_Card.hpp"
#include <chrono>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <string>
#include <string.h>
#include <fstream>

extern Network Azure;
//extern SDCard SD;
//extern MatrixGraph BarChart;
//extern Circular_Buff <sample_t> SampleBuffer;
//extern EventQueue PrintQueue;

//using namespace uop_msb;

Network::Network(){
    //connect(); //establish network connection
    //setTime(); //set the current time
    sendrate = 4000ms; //default sendrate 4s
}

bool message_received = false;

void Network::SetSendRate(std::chrono::milliseconds rate) //set the rate at which samples are sent to azure
{
    sendrate = rate; //no mutex needed, this is set only once, long before it is ever accessed
}

std::chrono::milliseconds Network::GetSendRate() //acquire current sending rate
{
    return sendrate;
}




bool Network::connect()
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

bool Network::setTime()
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

time_t Network::getTime(){
    return timestamp;           //returns the current time for other classes to use
}


void Network::SendData() {
    bool trace_on = MBED_CONF_APP_IOTHUB_CLIENT_TRACE;
    tickcounter_ms_t interval = 100;
    IOTHUB_CLIENT_RESULT res;

    PrintQueue.call(printf, "Initializing IoT Hub client\n\r");
    IoTHub_Init();



    IOTHUB_DEVICE_CLIENT_HANDLE client_handle = IoTHubDeviceClient_CreateFromConnectionString(
        azure_cloud::credentials::iothub_connection_string,
        MQTT_Protocol
    );
    if (client_handle == nullptr) {
        PrintQueue.call(printf, "Failed to create IoT Hub client handle\n\r");
        goto cleanup;
    }

    // Enable SDK tracing
    res = IoTHubDeviceClient_SetOption(client_handle, OPTION_LOG_TRACE, &trace_on);
    if (res != IOTHUB_CLIENT_OK) {
        PrintQueue.call(printf, "Failed to enable IoT Hub client tracing, error: %d\n\r", res);
        goto cleanup;
    }

    // Enable static CA Certificates defined in the SDK
    res = IoTHubDeviceClient_SetOption(client_handle, OPTION_TRUSTED_CERT, certificates);
    if (res != IOTHUB_CLIENT_OK) {
        PrintQueue.call(printf, "Failed to set trusted certificates, error: %d\n\r", res);
        goto cleanup;
    }

    // Process communication every 100ms
    res = IoTHubDeviceClient_SetOption(client_handle, OPTION_DO_WORK_FREQUENCY_IN_MS, &interval);
    if (res != IOTHUB_CLIENT_OK) {
        PrintQueue.call(printf, "Failed to set communication process frequency, error: %d\n\r", res);
        goto cleanup;
    }

    // set incoming message callback
    res = IoTHubDeviceClient_SetMessageCallback(client_handle, on_message_received, nullptr);
    if (res != IOTHUB_CLIENT_OK) {
        PrintQueue.call(printf, "Failed to set message callback, error: %d\n\r", res);
        goto cleanup;
    }

    // Set incoming command callback
    res = IoTHubDeviceClient_SetDeviceMethodCallback(client_handle, on_method_callback, nullptr);
    if (res != IOTHUB_CLIENT_OK) {
        PrintQueue.call(printf, "Failed to set method callback, error: %d\n\r", res);
        goto cleanup;
    }

    // Set connection/disconnection callback
    res = IoTHubDeviceClient_SetConnectionStatusCallback(client_handle, on_connection_status, nullptr);
    if (res != IOTHUB_CLIENT_OK) {
        PrintQueue.call(printf, "Failed to set connection status callback, error: %d\n\r", res);
        goto cleanup;
    }

    // Send ten message to the cloud (one per second)
    // or until we receive a message from the cloud
    IOTHUB_MESSAGE_HANDLE message_handle;
    char message[200];
    while(1){ //while loops so it doesnt stop sending data and block
        if (message_received) {
            // If we have received a message from the cloud, don't send more messeges
            break;
        }
        //Send data in this format:
        /*
            {
                "LightLevel" : 0.12,
                "Temperature" : 36.0
            }

        */
        sample_t currSample = netBuff.get();
        sprintf(message, "{ \"date\" : \"%s\", \"LightLevel\" : %d, \"Pressure\" : %d, \"Temperature\" : %d, \"time\" : \"%s\" }", currSample.date, currSample.light, currSample.press, currSample.temp, currSample.time);
        PrintQueue.call(printf, "Sending: \"%s\"\n\r", message);

        message_handle = IoTHubMessage_CreateFromString(message);
        if (message_handle == nullptr) {
            PrintQueue.call(printf, "Failed to create message\n\r");
            goto cleanup;
        }

        res = IoTHubDeviceClient_SendEventAsync(client_handle, message_handle, on_message_sent, nullptr);
        IoTHubMessage_Destroy(message_handle); // message already copied into the SDK

        if (res != IOTHUB_CLIENT_OK) {
            PrintQueue.call(printf, "Failed to send message event, error: %d\n\r", res);
            goto cleanup;
        }

        //WatchdogCheck.kickNetworking = WatchdogCheck.ENABLED; //kicks the watchdog for the networking thread
        //WatchdogCheck.KICKnetworking(); //kicks the watchdog for the network thread
        ThisThread::sleep_for(sendrate);

    }

    // If the user didn't manage to send a cloud-to-device message earlier,
    // let's wait until we receive one
    /*
    while (!message_received) {
        // Continue to receive messages in the communication thread
        // which is internally created and maintained by the Azure SDK.
        sleep();
    }
    */
    
cleanup:
    IoTHubDeviceClient_Destroy(client_handle);
    IoTHub_Deinit();
}

int on_method_callback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* response_size, void* userContextCallback)
{
    //called when a message is received

    //DigitalOut led1(TRAF_RED1_PIN);
    //DigitalIn Button(BTN1_PIN);
    const char* device_id = (const char*)userContextCallback;

    /*
    printf("\r\nDevice Method called for device %s\r\n", device_id);
    printf("Device Method name:    %s\r\n", method_name);
    printf("Device Method payload: %.*s\r\n", (int)size, (const char*)payload);
    */
    
    char RESPONSE_STRING[200] = "\"Response:\""; //we will need this in a moment 
    std::string ThreshStr(reinterpret_cast<char const*>(payload), size); //for getting integers from the payload for thresholds
    if(strncmp("Plot", (const char*)method_name, size) == 0)
    {       
        if(strncmp("\"T\"", (const char*)payload, size) == 0)
        {
            Azure.setDisplayParam(TEMP); //display temperature on the bar chart
            PrintQueue.call(printf, "\n\r---PLOTTING TEMPERATURE---\n\r");
            sprintf(RESPONSE_STRING, "{ \"ConfirmDisplay\" : \"Temperature Plotted\" }");     

        }else if(strncmp("\"P\"", (const char*)payload, size) == 0)
        {    
            Azure.setDisplayParam(PRESS); //display press on the bar chart
            PrintQueue.call(printf, "\n\r---PLOTTING PRESSURE---\n\r");
            sprintf(RESPONSE_STRING, "{ \"ConfirmDisplay\" : \"Pressure Plotted\" }");
     
        }else if(strncmp("\"L\"", (const char*)payload, size) == 0)
        {    
            Azure.setDisplayParam(LIGHT); //display light on the bar chart
            PrintQueue.call(printf, "\n\r---PLOTTING LIGHT---\n\r");
            sprintf(RESPONSE_STRING, "{ \"ConfirmDisplay\" : \"Light Plotted\" }");
        }
        else {
            //syntax error in the command 
            PrintQueue.call(printf, "select variable with T, P or L\r\n");
            sprintf(RESPONSE_STRING, "{ \"ConfirmDisplay\" : \"Nothing Selected\" }");
        }
    }else if(strncmp("BufferCMD", (const char*)method_name, size) == 0)
    {
         
        if(strncmp("\"latest\"", (const char*)payload, 6) == 0)
        {
            //Fetch Latest Date, Time, Temperature, Pressure and Light Level stored in the FIFO buffer and return these in the response
            PrintQueue.call(printf, "\n\r---FETCHING LATEST---\n\r");
            sample_t currSample = SampleBuffer.peek(0);
            sprintf(RESPONSE_STRING, "{ \"BufferResponse\": \"Light : %d, Temp : %d, Press : %d, Date : %s, Time : %s\" }", currSample.light, currSample.temp, currSample.press, currSample.date, currSample.time);
            
        }else if(strncmp("\"buffered\"", (const char*)payload, 8) == 0)
        {
            //Read the number of samples currently in the buffer and return in the response
            PrintQueue.call(printf, "\n\r---READING NUM IN BUFFER---\n\r");
            size_t BuffSize = SampleBuffer.getSize();
            sprintf(RESPONSE_STRING, "{ \"BufferResponse\": \"Buffered Samples : %d\" }", BuffSize);
        }else if(strncmp("\"flush\"", (const char*)payload, 5) == 0)
        {
            PrintQueue.call(printf, "\n\r---ATTEMPTING TO FLUSH BUFFER TO SD---\n\r");
            int success = SD.flush();
            if (success == 0){
                sprintf(RESPONSE_STRING, "{ \"BufferResponse\": \"Buffer Flushed\" }");
            }else{
                sprintf(RESPONSE_STRING, "{ \"BufferResponse\": \"Buffer Failed to Flush\" }");
            }
            //Write all samples to the SD card and empty the buffer, responding with an acknowledgement
            
            
        }else
        {
            PrintQueue.call(printf, "\r\n----------NO COMMAND----------\r\n");
        }
    }else if(strncmp("Setlow", (const char*)method_name, 6) == 0){
        
        if (strncmp("SetlowTemp", (const char *)method_name, 10) == 0) 
        {
            // Set low temperature threshold for temperature, pressure and light,
            // responding with an acknowledgement
            unsigned int lowthresh = stoi(ThreshStr);
            Azure.setThresholds(lowthresh, TEMP, LOWER);
            
            PrintQueue.call(printf, "\n\r---SETTING LOWER TEMP THRESH---\n\r");
            sprintf(RESPONSE_STRING, "\"Lower Temp Thresh Set to : %d\"", lowthresh);

        } else if (strncmp("SetlowPress", (const char *)method_name, 11) == 0) 
        {

            // Set low temperature threshold for temperature, pressure and light,
            // responding with an acknowledgement
            unsigned int lowthresh = stoi(ThreshStr);
            Azure.setThresholds(lowthresh, PRESS, LOWER);
            PrintQueue.call(printf, "\n\r---SETTING LOWER PRESSURE THRESH---\n\r");
            sprintf(RESPONSE_STRING, "\"Lower Press Thresh Set to : %d\"", lowthresh);

        } else if (strncmp("SetlowLight", (const char *)method_name, 11) == 0) 
        {
            // Set low temperature threshold for temperature, pressure and light,
            // responding with an acknowledgement
            unsigned int lowthresh = stoi(ThreshStr);
            Azure.setThresholds(lowthresh, LIGHT, LOWER);
            PrintQueue.call(printf, "\n\r---SETTING LOWER LIGHT THRESH---\n\r");
            sprintf(RESPONSE_STRING, "\"Lower Light Thresh Set to : %d\"", lowthresh);

        }
    }else if(strncmp("Sethigh", (const char*)method_name, 6) == 0){
        
        if (strncmp("SethighTemp", (const char *)method_name, 10) == 0) 
        {
            // Set high temperature threshold for temperature, pressure and light,
            // responding with an acknowledgement
            unsigned int highthresh = stoi(ThreshStr);
            Azure.setThresholds(highthresh, TEMP, UPPER);
            PrintQueue.call(printf, "\n\r---SETTING UPPER TEMP THRESH---\n\r");
            sprintf(RESPONSE_STRING, "\"Upper Temp Thresh Set to : %d\"", highthresh);

        } else if (strncmp("SethighPress", (const char *)method_name, 11) == 0) 
        {
            // Set high temperature threshold for temperature, pressure and light,
            // responding with an acknowledgement
            unsigned int highthresh = stoi(ThreshStr);
            Azure.setThresholds(highthresh, PRESS, UPPER);
            PrintQueue.call(printf, "\n\r---SETTING UPPER PRESSURE THRESH---\n\r");
            sprintf(RESPONSE_STRING, "\"Upper Press Thresh Set to : %d\"", highthresh);

        } else if (strncmp("SethighLight", (const char *)method_name, 11) == 0) 
        {
            // Set high temperature threshold for temperature, pressure and light,
            // responding with an acknowledgement
            unsigned int highthresh = stoi(ThreshStr);
            Azure.setThresholds(highthresh, LIGHT, UPPER);
            PrintQueue.call(printf, "\n\r---SETTING UPPER LIGHT THRESH---\n\r");
            sprintf(RESPONSE_STRING, "\"Upper Light Thresh Set to : %d\"", highthresh);

        } 
    }
    else 
    {
        PrintQueue.call(printf, "\r\n\r\n---------NO COMMAND!---------\r\n\r\n");
    }
    
    int status = 200;
    //char RESPONSE_STRING[] = "{ \"Response\": \"This is the response from the device\" }";
    
    //sprintf(RESPONSE_STRING, "{ \"Response\" :  }");

    
    PrintQueue.call(printf, "\r\nResponse status: %d\r\n", status);
    //PrintQueue.call(printf, "Response payload: \"%s\"\n\r", RESPONSE_STRING); //this cant pass the string correctly for some reason?? works elsewhere, still sends the correct message
    //PrintQueue.call(printf, "Sending: \"%s\"\n\r", message);

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


void on_message_sent(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    if (result == IOTHUB_CLIENT_CONFIRMATION_OK) {
        PrintQueue.call(printf, "Message sent successfully\n\r");
    } else {
        PrintQueue.call(printf, "Failed to send message, error: \"%s\"\n\r",
            MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
    }
}

IOTHUBMESSAGE_DISPOSITION_RESULT on_message_received(IOTHUB_MESSAGE_HANDLE message, void* user_context)
{
    DigitalOut led2(TRAF_GRN1_PIN);
    PrintQueue.call(printf, "Message received from IoT Hub\n\r");

    const unsigned char *data_ptr;
    size_t len;
    if (IoTHubMessage_GetByteArray(message, &data_ptr, &len) != IOTHUB_MESSAGE_OK) {
        PrintQueue.call(printf, "Failed to extract message data, please try again on IoT Hub\n\r");
        return IOTHUBMESSAGE_ABANDONED;
    }

    message_received = true;
    //PrintQueue.call(printf, ("Message body: %.*s", len, data_ptr);
    PrintQueue.call(printf, "Message body: %.*s\r\n", len, data_ptr);

    if (strncmp("true", (const char*)data_ptr, len) == 0) {
        led2 = 1;
    }else if (strncmp("T", (const char*)data_ptr, len) == 0) {
        
        
        
    }else {
        led2 = 0;
    }

    

    return IOTHUBMESSAGE_ACCEPTED;
}

void on_connection_status(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context)
{
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED) {
        //PrintQueue.call(printf, ("Connected to IoT Hub");
        PrintQueue.call(printf, "Connected to IoT Hub\r\n");
    } else {
        PrintQueue.call(printf, "Connection failed, reason: \"%s\"\n\r", MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONNECTION_STATUS_REASON, reason));
    }
}