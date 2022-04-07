#ifndef __SDWRAPPER_HPP__
#define __SDWRAPPER_HPP__

#include "mbed.h"
#include "../mbed-os/storage/blockdevice/COMPONENT_SD/include/SD/SDBlockDevice.h" //:D


//#include "FATFileSystem.h"
#include "../mbed-os/storage/filesystem/fat/include/fat/FATFileSystem.h"
#include "Config.hpp"

extern EventQueue PrintQueue;


class SDCARD 
{

    public:

        SDCARD(SPIConfig_t pins);
        ~SDCARD();
        void Test(void);

    private:


    SDBlockDevice SD;
    FATFileSystem fs;
    FILE *fp;

};





#endif

