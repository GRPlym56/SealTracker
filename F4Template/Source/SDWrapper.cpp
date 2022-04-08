#include "SDWrapper.hpp"

SDCARD::SDCARD(SPIConfig_t pins): SD(pins.MOSI, SDpins.MISO, SDpins.SCLK, SDpins.CS), fs("sd", &SD)
{

    //FATFileSystem fs("sd", &SD);
}

void SDCARD::Test(void)
{
    int err = SD.init();
    if ( 0 != err) {
        PrintQueue.call(printf, "SD Startup failed %d\n",err);
    }
    
    //FATFileSystem fs("sd", &SD);
    fp = fopen("/sd/test.txt","w");
    if(fp == NULL) {
        PrintQueue.call(printf, ("Could not open file for write\n"));
        SD.deinit();
        //return -1;
    } else {
        //Put some text in the file...
        fprintf(fp, "Blubbery Seals\n");
        //Tidy up here
        fclose(fp);
        PrintQueue.call(printf, ("SD Write done...\n"));
        SD.deinit();
        //return 0;
    }
}

int SDCARD::WriteSample(void)
{
    int err = SD.init();
    if ( 0 != err) {
        PrintQueue.call(printf, "SD Startup failed %d\n",err);
        return -1;
    }

    char fileName[20];
    sprintf(fileName, "Samples.csv");
    fsmp = fopen(fileName, "w");

    if(fp == NULL)
    {
        error("Could not open file for write\n");
        SD.deinit();
        return -1;
    }else 
    {
        /*
        TODO
        output =  something.getsample();
        fprintf(fp, "%s", output.c_str());
        */
        return 0;
    }

}