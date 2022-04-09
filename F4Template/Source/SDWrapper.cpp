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

int SDCARD::flush() //flush all samples from buffer to 
{
    PrintQueue.call(printf, "Flushing to SD\n");
    int err;
    // call the SDBlockDevice instance initialisation method.

    err = SD.init();
    if ( 0 != err) {
        PrintQueue.call(printf, "Init failed %d\n",err);
        return -1;
    }
    
    //FATFileSystem fs("sd", &sd);
    char fileName[40];
    /*
    time_t timestamp = time(NULL); //update current time
    tm *ltm = localtime(&timestamp); //format time into sec, min, hrs, days, months, years

    sprintf(fileName, "/sd/Samples_%04d-%02d-%02d_%02d-%02d-%02d.csv", 1900+ltm->tm_year, 1+ltm->tm_mon, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    */
    sprintf(fileName, "/sd/Samples.csv");
    PrintQueue.call(printf, "%s\n", fileName);

    sealsample_t output;

    fp = fopen(fileName,"w");
    if(fp == NULL) {
        error("Could not open file for write\n");
        SD.deinit();
        return -1;
    } else {
        //SDLock.lock();
        //Put some text in the file...
        unsigned int size = SampleBuffer.GetSize();
        for(int i = 0; i<size; i++) //loop for as many samples as we have
        { 
            output = SampleBuffer.Get();
            //PrintQueue.call(printf, "Write to SD: %s, %s\n\r", output.pressure.c_str(), output.temperature.c_str()); //write each n byte line
            fprintf(fp, "P: %s", output.pressure.c_str()); //write each n byte line
            fprintf(fp, "T: %s", output.temperature.c_str());
            
        }
        //SDLock.unlock();
        //Tidy up here
        fclose(fp);
        PrintQueue.call(printf, "SD Write done...\n");
        SD.deinit();
        return 0;
    }
}