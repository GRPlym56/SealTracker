/*
    Written by Guy Ringshaw 2022
*/

#include "mbed.h"
#include "platform/mbed_thread.h"
#include "nRF24L01P.h"
#include "MS5837.h"
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


CircBuff SDBuffer(256, "SDBuff"); //main buffer for getting samples to the SD card
SDCARD microSD(SDpins, &SDBuffer);

MS5837 PressSens(PA_10, PA_9); //SDA, SCL
CommsWrapper NRF(RFPINS);

CircBuff DiveBuff(128, "DiveBuff"); //secondary buffer for live readout of data characterising dive behaviour on azure
SealSubmersion DiveTracker(&DiveBuff, &NRF, &PressSens); 




Thread SealSumbersion, PrintThread, SDThread, SamplerThread, BufferThread;
osThreadId_t BuffId;

void UpdateSamplers();
void Printer();
void SDFlush();
void UpdateBuffers();


int main() {

    //set_time(1651322988); //unix epoch time
    

    PrintThread.start(Printer); //must start before everything else
    microSD.Test();

    SDThread.start(SDFlush);
    SDThread.set_priority(osPriorityLow); //low priority 
    BufferThread.start(UpdateBuffers);
    ThisThread::sleep_for(100ms);
    SamplerThread.start(UpdateSamplers);

    
    //BuffId = BufferThread.get_id();

    if(sleep_manager_can_deep_sleep()) //check deep sleep
    {
        PrintQueue.call(printf,  "deep sleep available \n\r");
    }
    
    
    while(1) 
    {
        
        DiveTracker.SurfaceDetection(); 
        
        
    }

}


void UpdateSamplers()
{
    
    while(1)
    {  
        PressSens.Barometer_MS5837();
        osSignalSet(BuffId, SampleFlag); //synchronise buffer updates with sampling
        ThisThread::sleep_for(10s);
    }
}

void UpdateBuffers()
{
    BuffId = ThisThread::get_id();
    unsigned short count = 0;
    while(1)
    {
        sealsampleL4_t sample;
        //char timesample[32];
         
        ThisThread::flags_wait_any(SampleFlag, true); //wait for flag, and clear
        //ThisThread::sleep_for(15s);
        sample.pressure = PressSens.MS5837_Pressure();
        sample.temperature = PressSens.MS5837_Temperature();
        PrintQueue.call(printf, "P:%f, T:%s\n\r", sample.pressure, sample.temperature);
        //sample.time = strftime(timesample, 32, "%b,%d,%H:%M", localtime(&seconds));
        SDBuffer.Put(sample); //put new sample on buffer
        
        count++;
        if(count == 5) 
        {
            DiveBuff.Put(sample); //update dive characteristic buffer every 5 mins
            count = 0;
        }
        
    }
}



void Printer()
{
    PrintQueue.call(printf, "starting printer\n");
    PrintQueue.dispatch_forever();
}

void SDFlush()
{
    while(1)
    {
        microSD.ManageSD(); //flushes samples from buffer to SD after set period
    }
}


