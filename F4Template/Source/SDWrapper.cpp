#include "SDWrapper.hpp"

SDCARD::SDCARD(SPIConfig_t pins, CircBuff* SDBuff): SD(pins.MOSI, SDpins.MISO, SDpins.SCLK, SDpins.CS), fs("sd", &SD), SDBuffer(SDBuff)
{

    //FATFileSystem fs("sd", &SD);

    //write some information at the top of the files
    

    int err;
    err = SD.init();
    if ( 0 != err) {
        PrintQueue.call(printf, "Init failed %d\n",err);
        //return -1;
    }
    
    PrintQueue.call(printf, "%s\n", fileName);
    //PrintQueue.call(printf, "%s\n", fileName1);
    /*
    fp = fopen(fileName,"w");
    ft = fopen(fileName1, "w");
    */
    samplefile = fopen(fileName, "w");

    if(samplefile == NULL) {
        error("Could not open file for write\n");
        SD.deinit();
        
    }else
    {
    
        //PrintQueue.call(printf, "Write to SD: %s, %s\n\r", output.pressure.c_str(), output.temperature.c_str()); //write each n byte line
        fprintf(samplefile, "Pressure mBar,Temperature C, Time\n"); 
        
        //SDLock.unlock();
        //Tidy up here
        fclose(samplefile);
        PrintQueue.call(printf, "SD Write done...\n");
        SD.deinit();
        
    }
    
}

void SDCARD::Test(void)
{
    int err = SD.init();
    if ( 0 != err) {
        PrintQueue.call(printf, "SD Startup failed %d\n",err);
    }
    
    //FATFileSystem fs("sd", &SD);
    ftest = fopen("/sd/test.txt","w");
    if(ftest == NULL) {
        PrintQueue.call(printf, ("Could not open file for write\n"));
        SD.deinit();
        //return -1;
    } else {
        //Put some text in the file...
        fprintf(ftest, "Blubbery Seals\n");
        //Tidy up here
        fclose(ftest);
        PrintQueue.call(printf, ("SD Write done...\n"));
        SD.deinit();
        //return 0;
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
    
    /*
    time_t timestamp = time(NULL); //update current time
    tm *ltm = localtime(&timestamp); //format time into sec, min, hrs, days, months, years

    sprintf(fileName, "/sd/Samples_%04d-%02d-%02d_%02d-%02d-%02d.csv", 1900+ltm->tm_year, 1+ltm->tm_mon, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    */
    
    //sprintf(fileName, "/sd/P-Samples.csv");
    //sprintf(fileName1, "/sd/T-Samples.csv");
    
    PrintQueue.call(printf, "%s\n", fileName);
    //PrintQueue.call(printf, "%s\n", fileName1);
    sealsample_t output;

    /*
    fp = fopen(fileName,"w");
    ft = fopen(fileName1, "w");
    */
    samplefile = fopen(fileName, "ab");
    if(samplefile == NULL) {
        error("Could not open file for write\n");
        SD.deinit();
        return -1;
    }else
    {
        
        //Put some data in the file...
        unsigned int size = SDBuffer->GetSize();
        for(int i = 0; i<size; i++) //loop for as many samples as we have
        { 
            output = SDBuffer->Get();
            //PrintQueue.call(printf, "Write to SD: %s, %s\n\r", output.pressure.c_str(), output.temperature.c_str()); //write each n byte line
            /*
            fprintf(fp, "%s\n", output.pressure.c_str()); //write each n byte line
            fprintf(ft, "%s\n", output.temperature.c_str());
            */
            
            fprintf(samplefile, "%s,%s\n", output.pressure.c_str(), output.temperature.c_str(), output.time.c_str());
            
        }
        //SDLock.unlock();
        //Tidy up here
        /*
        fclose(fp);
        fclose(ft);
        */
        fclose(samplefile);
        PrintQueue.call(printf, "SD Write done...\n");
        SD.deinit();
        return 0;
    }
}