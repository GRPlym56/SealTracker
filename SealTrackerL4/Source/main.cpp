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




EventQueue PrintQueue;


MS5837 PressSens(PA_10, PA_9); //SDA, SCL
CommsWrapper NRF(RFPINS);

CircBuff SDBuffer(512, "SDBuff"); //main buffer for getting samples to the SD card
SDCARD microSD(SDpins, &SDBuffer);

CircBuff DiveBuff(128, "DiveBuff"); //secondary buffer for live readout of data characterising dive behaviour on azure
SealSubmersion DiveTracker(&DiveBuff, &NRF, &PressSens); 




Thread SealSumbersion, PrintThread, SDThread;

void UpdateSamplers();
void Printer();
void SDFlush();



int main() {

    set_time(1651322988); //unix epoch time
    

    PrintThread.start(Printer); //must start before everything else
    microSD.Test();

    SDThread.start(SDFlush);
    SDThread.set_priority(osPriorityLow); //low priority 

    /*
        MAKE SURE BUILD PROFILE IN MBED STUDIO IS SET TO DEVELOP
        this reduces average current by approx 6mA
    */
    
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
    sealsample_t sample;
    char timesample[32];
    unsigned short count = 0;
    while(1)
    {
        //time is necessary as these will be viewed from the SD card later
        time_t seconds = time(NULL);
        PressSens.Barometer_MS5837();
        sample.pressure = PressSens.MS5837_Pressure();
        sample.temperature = PressSens.MS5837_Temperature();
        sample.time = strftime(timesample, 32, "%b,%d,%H:%M", localtime(&seconds));
        SDBuffer.Put(sample); //put new sample on buffer
        
        count++;
        if(count == 5) 
        {
            DiveBuff.Put(sample); //update dive characteristic buffer every 5 mins
        }
        ThisThread::sleep_for(60s);
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

