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
CircBuff SDBuffer(256, "SDBuff"); //main buffer for getting samples to the SD card
SDCARD microSD(SDpins, &SDBuffer);

MS5837 PressSens(PA_10, PA_9); //SDA, SCL


CircBuff DiveBuff(128, "DiveBuff"); //secondary buffer for live readout of data characterising dive behaviour on azure
SealSubmersion DiveTracker(&DiveBuff, &NRF, &PressSens); 


Thread PrintThread, SDThread, SamplerThread;


void UpdateSamplers();
void Printer();
void SDFlush();


int main() {

    set_time(1652184653); //12:10 may 10th 2022

    PrintThread.start(Printer); //should start before every other thread
    microSD.Test();
    NRF.InitSendNode();
    PressSens.MS5837Init();
    DiveTracker.GetAmbientDepth();
    

  

    //microSD.Test();

    SDThread.start(SDFlush);
    SDThread.set_priority(osPriorityLow); //low priority 

    
    SamplerThread.start(UpdateSamplers);

    
   

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
    volatile unsigned short count = 0;
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
        SDBuffer.Put(sample); //put new sample on buffer

        count++;
        PrintQueue.call(printf, "--Count: %d--\r\n", count);
        if(count == 5) 
        {
            DiveBuff.Put(sample); //update dive characteristic buffer every 5 mins
            count = 0;
    
        }
        ThisThread::sleep_for(5s);
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


