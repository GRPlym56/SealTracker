/*
ELEC-351 Group E
Author: Jacob Howell, Ethan Brian
December-January 2022
*/
#include "SD_Card.hpp"
#include <cstdint>
#include <cstdio>



// Instantiate the SDBlockDevice by specifying the SPI pins connected to the SDCard
// socket. The PINS are: (This can also be done in the JSON file see mbed.org Docs API Storage SDBlockDevice)
// PB_5    MOSI (Master Out Slave In)
// PB_4    MISO (Master In Slave Out)
// PB_3    SCLK (Serial Clock)
// PF_3    CS (Chip Select)
//
// and there is a Card Detect CD on PF_4 ! (NB this is an Interrupt capable pin..)
//SDBlockDevice sd(SDpins.MOSI, SDpins.MISO, SDpins.SCLK, SDpins.CS);

SDCard::SDCard(SPIConfig_t pins):   sd(pins.MOSI, pins.MISO, pins.SCLK, pins.CS),
                                    fs("sd", &sd),
                                    SDBuff(128, "SD Buffer"){
    SDLock.lock();
    
    sprintf(sample,"Date,\tTime,\tLight,\tTemp,\tPress,\t\r\n"); //write first line to CSV file
    SDBuff.put(sample);
    SDLock.unlock();
}

void SDCard::setConsumerContext(ConsumerContext* Context){
    //do nothing because SD doesn't care about the context
}
void SDCard::consumeSample(sample_t currSample){
    SDLock.lock();

    if(SDBuff.getSize() == 0){
        sprintf(sample,"Date,\tTime,\tLight,\tTemp,\tPress,\t\r\n"); //write first line to CSV file
        SDBuff.put(sample);
    }
    if(!SDBuff.isFull()){
        sprintf(sample, "%s,\t%s,\t%d,\t%d,\t%d,\t\r\n", currSample.date, currSample.time, currSample.light, currSample.temp, currSample.press);
        SDBuff.put(sample);
    }
    SDLock.unlock();
}
int SDCard::buffFull(){
    return SDBuff.isFull();
}
int SDCard::buffEmpty(){
    return SDBuff.isEmpty();
}
int SDCard::getSize(){
    return SDBuff.getSize();
}

int SDCard::getCapacity(){
    return SDBuff.getCapacity();
}
float SDCard::getUsage(){
    return ((float)SDBuff.getSize()/(float)SDBuff.getCapacity());
}
int SDCard::flush()
{
    PrintQueue.call(printf, "Initialise and write to a file\n");
    int err;
    // call the SDBlockDevice instance initialisation method.

    err = sd.init();
    if ( 0 != err) {
        PrintQueue.call(printf, "Init failed %d\n",err);
        return -1;
    }
    
    //FATFileSystem fs("sd", &sd);
    char fileName[40];
    time_t timestamp = time(NULL); //update current time
    tm *ltm = localtime(&timestamp); //format time into sec, min, hrs, days, months, years

    sprintf(fileName, "/sd/Samples_%04d-%02d-%02d_%02d-%02d-%02d.csv", 1900+ltm->tm_year, 1+ltm->tm_mon, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    PrintQueue.call(printf, "%s", fileName);

    fp = fopen(fileName,"w");
    if(fp == NULL) {
        error("Could not open file for write\n");
        sd.deinit();
        return -1;
    } else {
        SDLock.lock();
        //Put some text in the file...
        while(!SDBuff.isEmpty()){
            output = SDBuff.get();
            PrintQueue.call(printf, "%s", output.c_str()); //write each n byte line
            fprintf(fp, "%s", output.c_str()); //write each n byte line
        }
        SDLock.unlock();
        //Tidy up here
        fclose(fp);
        PrintQueue.call(printf, "SD Write done...\n");
        sd.deinit();
        return 0;
    }
    
}

int SDCard::read()
{
    PrintQueue.call(printf, "Initialise and read from a file\n");

    // call the SDBlockDevice instance initialisation method.
    if ( 0 != sd.init()) {
        PrintQueue.call(printf, "Init failed \n");
        return -1;
    }
    
    //FATFileSystem fs("sd", &sd);
    fp = fopen("/sd/samples.csv","r");
    if(fp == NULL) {
        error("Could not open or find file for read\n");
        sd.deinit();
        return -1;
    } else {
        //Put some text in the file...
        char buff[64]; buff[63] = 0;
        while (!feof(fp)) {
            fgets(buff, 63, fp);
            PrintQueue.call(printf, "%s\n", buff);
        }
        //Tidy up here
        fclose(fp);
        PrintQueue.call(printf, "SD Read done...\n");
        sd.deinit();
        return 0;
    }
}

void SDCard::test()
{
    //Test SD Card
    #ifdef USE_SD_CARD
    flush();
    read();
    #endif
}
