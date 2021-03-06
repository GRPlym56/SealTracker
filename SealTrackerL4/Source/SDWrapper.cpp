#include "SDWrapper.hpp"

SDCARD::SDCARD(SPIConfig_t pins, CircBuff* SDBuff): SD(SDpins.MOSI, SDpins.MISO, SDpins.SCLK, SDpins.CS), fs("sd", &SD), SDBuffer(SDBuff)
{
   
    int err;
    err = SD.init();
    if ( 0 != err) {
        PrintQueue.call(printf, "Init failed %d\n",err);
    }
    
    PrintQueue.call(printf, "%s\n", fileName);
    
    samplefile = fopen(fileName, "w");

    if(samplefile == NULL) {
        error("Could not open test file for write\n");
        SD.deinit();
        
    }else
    {
    
        fprintf(samplefile, "Pressure mbar ,Temperature C, Time, State\n"); 
         
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
        PrintQueue.call(printf, ("Could not open text file for write\n"));
        SD.deinit();
        //return -1;
    } else {
        //Put some text in the file...
        fprintf(ftest, "This is a text file\n");
        //Tidy up here
        fclose(ftest);
        PrintQueue.call(printf, ("SD Write done...\n"));
        SD.deinit();
        //return 0;
    }

    
}

int SDCARD::flush() //flush all samples from buffer to SD
{

        PrintQueue.call(printf, "Flushing to SD\n");
        int err;
        // call the SDBlockDevice instance initialisation method.

        err = SD.init();
        if ( 0 != err) {
            PrintQueue.call(printf, "Init failed %d\n",err);
            return -1;
        }
        
        
        PrintQueue.call(printf, "%s\n", fileName);
        //PrintQueue.call(printf, "%s\n", fileName1);
        sealsampleL4_t output;

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

                
                fprintf(samplefile, "%f,%f,%s,%d\n", output.pressure, output.temperature, output.time.c_str(), output.state);
                
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

void SDCARD::ManageSD() //to run in its own low priority thread
{

    ThisThread::sleep_for(60s); //sleep for 5 mins, less when testing     
    flush(); // flush samples to SD
    
}

osThreadId_t SDCARD::getThreadID()
{
    return ThisThread::get_id();
}