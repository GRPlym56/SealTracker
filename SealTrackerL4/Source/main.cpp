/*
    Written by Guy Ringshaw 2022
*/

#include "mbed.h"
#include "platform/mbed_thread.h"
#include "nRF24L01P.h"
#include "../MS5837/MS5837.h"
#include "string.h"
#include "SDWrapper.hpp"
#include "CBUFF.hpp"
#include "Config.hpp"
#include "CommsWrapper.hpp"
#include "SealSubmersion.hpp"

 /*
    MAKE SURE BUILD PROFILE IN MBED STUDIO IS SET TO DEVELOP    
 */


#define SampleFlag 1


EventQueue PrintQueue;

CommsWrapper NRF(RFPINS);
CircBuff MainSDBuffer(512, "MainSDBuff"); //main buffer for getting samples to the SD card
SDCARD microSD(SDpins, &MainSDBuffer);

MS5837 PressSens(PA_10, PA_9); //SDA, SCL


CircBuff ChrDiveBuff(512, "ChrDiveBuff"); //secondary buffer for live readout of data characterising general dive behaviour on azure
SealSubmersion DiveTracker(&ChrDiveBuff, &NRF, &PressSens); 


Thread PrintThread, SDThread, SamplerThread, DiveThread;
//Thread PrintThread(osPriorityNormal, OS_STACK_SIZE, nullptr, "Print Thread");



void UpdateSamplers();
void Printer();
void SDFlush();
void Dive();



int main() {

    

    PrintThread.start(Printer); //should start before every other thread
    
    microSD.Test();
    NRF.InitSendNode();

    NRF.RequestTime(); //get time from F429 and setup RTC

    NRF.InitSendNode(); //reinit send mode

    PressSens.MS5837Init();
    DiveTracker.GetAmbientDepth();

    //test loop
   

    DiveThread.start(Dive);
    DiveThread.set_priority(osPriorityAboveNormal);

  

    //microSD.Test();

    //SDThread.start(SDFlush);
    //SDThread.set_priority(osPriorityLow); //low priority 

    
    SamplerThread.start(UpdateSamplers);

    
   

    if(sleep_manager_can_deep_sleep()) //check deep sleep
    {
        PrintQueue.call(printf,  "deep sleep available \n\r");
    }
    
    
    while(1) 
    {
        
        //DiveTracker.SurfaceDetection(); 
        microSD.ManageSD();
        
    }

}


void UpdateSamplers()
{
    volatile unsigned int count = 0;
    while(1)
    {  
        sealsampleL4_t sample;
        char timesample[32];
        time_t seconds = time(NULL); //get current time from the rtc
        PressSens.Barometer_MS5837(); //get latest temperature and pressure values
        
        ThisThread::sleep_for(5ms); //give time for the conversion to finish
        sample.pressure = PressSens.MS5837_Pressure(); 
        sample.temperature = PressSens.MS5837_Temperature();

        PrintQueue.call(printf, "P:%f, T:%f\n\r", sample.pressure, sample.temperature);
        strftime(timesample, 32, "%b:%d:(%H:%M)", localtime(&seconds));
        PrintQueue.call(printf, "Time: %s\n\r", timesample);
        sample.time = timesample;
        sample.state = DiveTracker.GetSealState();
        MainSDBuffer.Put(sample); //put new sample on buffer
        ChrDiveBuff.Put(sample);
        /*
        count++;
        if(count >= 3)
        {
            ChrDiveBuff.Put(sample); //update dive characteristic buffer
            count = 0;
        }
        */
       
        ThisThread::sleep_for(10s);
    }
}

void Printer()
{
    PrintQueue.call(printf, "starting printer\n");
    while(1)
    {
        PrintQueue.dispatch_once();
        ThisThread::sleep_for(25ms);
    }
}

void SDFlush()
{
    while(1)
    {
        microSD.ManageSD(); //flushes samples from buffer to SD after set period
    }
}

void Dive()
{
    while(1)
    {
        DiveTracker.SurfaceDetection();
    }
}




