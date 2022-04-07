#include "SDWrapper.hpp"

SDCARD::SDCARD(SPIConfig_t pins): SD(pins.MOSI, SDpins.MISO, SDpins.SCLK, SDpins.CS), fs("sd", &SD)
{


}

void SDCARD::Test(void)
{
    int err = SD.init();
    if ( 0 != err) {
        PrintQueue.call(printf, "SD Startup failed %d\n",err);
    }
    
    FATFileSystem fs("sd", &SD);
    FILE *fp = fopen("/sd/test.txt","w");
    if(fp == NULL) {
        error("Could not open file for write\n");
        SD.deinit();
        //return -1;
    } else {
        //Put some text in the file...
        fprintf(fp, "SD TEST\n");
        //Tidy up here
        fclose(fp);
        printf("SD Write done...\n");
        SD.deinit();
        //return 0;
    }
}