/*

ELEC-351 Group E
Authors: Jacob Howell, Guy Ringshaw, Ethan Brian
December-January 2022

Copyright (c) 2020 Arm Limited
SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "uop_msb.h"
#include "rtos/ThisThread.h"
#include <cstring>
#include <string.h>
#include "network.hpp"
#include "cbuff.hpp"
#include "Matrix.hpp"
#include "SD_Card.hpp"
#include <vector>
#include "sampleReceiver.hpp"
#include "NetworkSampleConsumer.hpp"
#include "Matrix.hpp"
#include "Config.hpp"
#include "nRF24L01P.h"
//#define TIMEOUT_MS 30000

using namespace uop_msb;

nRF24L01P Comms(PA_7, PA_6, PA_5, PE_11, PF_13, PE_9);
DigitalOut myled(LED2);

Thread Producer, Consumers, Graph, Networking, SD_writer, WatchdogThread, SerialThread;


Network Azure;
NetworkSampleConsumer AzureConsumer(&Azure);
Circular_Buff <sample_t> SampleBuffer(512, "Sample Buffer");
SDCard SD(SDpins);
MatrixGraph BarChart(MatrixPins);
EventQueue PrintQueue;


vector<SampleConsumer*> sampleConsumers; 
void registerSampleConsumers();
void sampler();
void updateConsumers();
void grapher();
void networking();

void SD_Check();

void WatchdogChecker();

void Printer();


int main() {
    //bool Connection = connect(); //variable there so it shuts up, maybe you want to flash an LED or something when it connects IDK
    //printf("\r\n---------------in main--------------\r\n");
    //Watchdog &watchdog = Watchdog::get_instance();
    //watchdog.start(TIMEOUT_MS); 

    /*
    registerSampleConsumers();
    Producer.start(sampler);
    Producer.set_priority(osPriorityRealtime7);
    Consumers.start(updateConsumers);
    //Sensors.setSampleRate(10s);
    Azure.SetSendRate(10000ms);
    Networking.start(networking);
    //Networking.set_priority(osPriorityLow);
    Graph.start(grapher);
    Graph.set_priority(osPriorityRealtime);


    SD_writer.start(SD_Check);
    */
    //WatchdogThread.start(WatchdogChecker); // starts the watchdog thread
    //WatchdogThread.set_priority(osPriorityRealtime1); // sets the priority of the watchdog thread to highest

    SerialThread.start(Printer);

    char txData[32], rxData[32];
    int txDataCnt = 0;
    int rxDataCnt = 0;



    Comms.powerUp();
    Comms.powerDown();
    Comms.powerUp();

    Comms.setTransferSize(32);
 
    Comms.setReceiveMode();
    Comms.enable();
    while (true) {
        if ( Comms.readable() ) {
 
            // ...read the data into the receive buffer
            rxDataCnt = Comms.read( NRF24L01P_PIPE_P0, rxData, sizeof( rxData ) );
 
            // Display the receive buffer contents via the host serial link
            for ( int i = 0; rxDataCnt > 0; rxDataCnt--, i++ ) {

                char a = rxData[i];
                printf("%c", a);
            }
 
            // Toggle LED2 (to help debug nRF24L01+ -> Host communication)
            myled = !myled;
        }
    }
    Producer.join();
    Consumers.join();
    //Networking.join();
    Graph.join();
}


void registerSampleConsumers(){
    SD.setConsumerContext(&Azure);          //initalise SD context
    sampleConsumers.push_back(&SD);               //register SD as a receiver

    //WarningAlarm.setConsumerContext(&Azure); //initalise sampleAlarm
    //sampleConsumers.push_back(&WarningAlarm);      //register sampleAlarm as receiver


    AzureConsumer.setConsumerContext(&Azure);
    sampleConsumers.push_back(&AzureConsumer);  //TODO STOP THIS FROM BLOCKING no, i dont think I will
    
    BarChart.setConsumerContext(&Azure);
    sampleConsumers.push_back(&BarChart);
}

void updateConsumers(){
    volatile int numConsumers = sampleConsumers.size();
    PrintQueue.call(printf, "Num Consumers: %d\r\n", numConsumers);
    while(1){

        while(!SampleBuffer.isEmpty()){             //if sample buffer is empty, block recieve data
            //PrintQueue.call(printf, "\r\nUPDATING CONSUMERS\r\n");
            sample_t sample = SampleBuffer.get();

            if (sample.valid){                          //check if sample is valid before updating consumers
                for(int i = 0; i < numConsumers; i++){  //itterate through list of recievers
                    //Receivers[i]->consumeSample(sample);  //call recieveData function for each receiver
                    SampleConsumer* R = sampleConsumers[i];
                    if(R != nullptr){
                        //PrintQueue.call(printf, "Calling Receive\r\n");
                        R->consumeSample(sample);           //put sample on consumers internal buffers
                    }else{
                        PrintQueue.call(printf, "Receiver is null\r\n");
                    }
                }
            }else{
                PrintQueue.call(printf, "Sample Invalid: %d\r\n", sample.valid);
            }

        //TODO Add other receive data functions
        }
        //WatchdogCheck.kickConsumers = WatchdogCheck.ENABLED; //kicks the watchdog for the consumer thread
        ThisThread::sleep_for(1s); //  change the sleep to 25 seconds to simulate a deadlock for the watchdog function
    }
}

//SD FIFO Hook calls custom function when gets to threshold
void SD_Check(){
    while(1){
        //if(SD.getSize() >= (SD.getCapacity()) - 10){ //if the SD Buffer is almost full, write all samples to SD
        if(SD.getUsage() >= 0.8f){  //if usage is above 80%, auto flush buffer to SD card    
            SD.flush();
        }
        ThisThread::sleep_for(5s);
    }
}


void grapher(){
    PrintQueue.call(printf, "Starting GraphWrite\r\n");
    BarChart.GraphWrite();
    //sleep();
}

void networking(){
    PrintQueue.call(printf, "Starting Networking\r\n");
    Azure.SendData(); //function contains a while loop - should never exit
}

void Printer()
{   
    PrintQueue.call(printf, "Starting Printer\n\r");
    PrintQueue.dispatch_forever();
}
