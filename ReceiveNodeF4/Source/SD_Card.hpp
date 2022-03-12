/*
ELEC-351 Group E
Authors: Jacob Howell, Ethan Brian
December-January 2022
*/
#ifndef __SD_CARD__
#define __SD_CARD__
#include "mbed.h"
#include "SDBlockDevice.h"
#include "FATFileSystem.h"
#include "Config.hpp"
#include "sampleReceiver.hpp"
#include "cbuff.hpp"
#include <string>
// SD Support?
#if (MBED_CONF_RTOS_PRESENT == 1)
#if (MBED_CONF_SD_FSFAT_SDCARD_INSTALLED == 1)
#define USE_SD_CARD
#define BLOCK_SIZE 64
#define LINE_WIDTH 40
#endif
#endif
//            "target.components_add": ["SD"],  
extern EventQueue PrintQueue;

class SDCard: public SampleConsumer
{

    public:
        //SDCard();
        SDCard(SPIConfig_t pins);                                       //initialise SDCard with SPI pins
        ~SDCard();                                                      //delete any data on heap, close comunications with SD
        void setSPI_Config(SPIConfig_t pins);                           //Configure SPI for comunication with SD Card
        void test();                                    
        int flush();                                                    //write all buffered data to the SD and respond with ack
        int read();                                                     //read data from SD Card
        int getSize();                                                  //returns current number of samples in buffer
        int getCapacity();                                              //returns maximum value of samples that can be stored in buffer
        float getUsage();                                               //return usage as percentage (from 0 to 1)
        int buffFull();                                                 //RETURNS TRUE IF BUFFER IS FULL
        int buffEmpty();                                                //returns true if buffer is empty
        void setConsumerContext(ConsumerContext* Context) override;     //update enviroment
        void consumeSample(sample_t sample) override;                   //format sample as CSV string and add to output buffer
    
    private:
        char sample[128];
        string output;
        Circular_Buff<string> SDBuff;
        Mutex SDLock;
        SDBlockDevice sd;
        FATFileSystem fs;
        FILE *fp;
};


#endif
