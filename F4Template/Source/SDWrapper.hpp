#ifndef __SDWRAPPER_HPP__
#define __SDWRAPPER_HPP__

#include "mbed.h"
#include "CBUFF.hpp"
#include "../mbed-os/storage/blockdevice/COMPONENT_SD/include/SD/SDBlockDevice.h" //:D


//#include "FATFileSystem.h"
#include "../mbed-os/storage/filesystem/fat/include/fat/FATFileSystem.h"
#include "Config.hpp"


#if (MBED_CONF_RTOS_PRESENT == 1)
#if (MBED_CONF_SD_FSFAT_SDCARD_INSTALLED == 1)
#define USE_SD_CARD
#define BLOCK_SIZE 64
#define LINE_WIDTH 40
#endif
#endif

extern EventQueue PrintQueue;
extern CircBuff SampleBuffer;

class SDCARD 
{

    public:

        SDCARD(SPIConfig_t pins);
        ~SDCARD();
        void Test(void);
        int WriteSample(void);

    private:


    SDBlockDevice SD;
    FATFileSystem fs;
    FILE *fp;
    FILE *fsmp;

};





#endif

