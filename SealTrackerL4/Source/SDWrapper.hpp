/*
    written by Guy Ringshaw 2022
*/

#ifndef __SDWRAPPER_HPP__
#define __SDWRAPPER_HPP__

#include "mbed.h"
#include "CBUFF.hpp"
#include "SDBlockDevice.h" //:D


//#include "FATFileSystem.h"
#include "FATFileSystem.h"
#include "Config.hpp"


#if (MBED_CONF_RTOS_PRESENT == 1)
#if (MBED_CONF_SD_FSFAT_SDCARD_INSTALLED == 1)
#define USE_SD_CARD
#define BLOCK_SIZE 64
#define LINE_WIDTH 40
#endif
#endif

extern EventQueue PrintQueue;


class SDCARD 
{

    public:

        SDCARD(SPIConfig_t pins, CircBuff* SDBuff);
        ~SDCARD();
        void Test(void);
        int flush();
        CircBuff* SDBuffer;
        void ManageSD();
        osThreadId_t getThreadID();

    private:


    SDBlockDevice SD;
    FATFileSystem fs;
    FILE *ftest;
    FILE *fsmp;
    FILE *samplefile; //file for samples
   
    

    char fileName[20] = "/sd/Samples.csv";
    
    

};





#endif

