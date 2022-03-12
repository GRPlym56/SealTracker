/*
ELEC-351 Group E
Author: Guy Ringshaw
December-January 2022
*/
#include "Matrix.hpp"
#include <cstdint>

MatrixGraph::MatrixGraph(SPIConfig_t Pins): BuffMatrix(8, "Matrix Buffer"), MatrixCMD(Pins.MOSI, Pins.MISO, Pins.SCLK), CS(Pins.CS), OE(Pins.OE)
{

    sample_t emptySample{1,0,0,0,0};
    for (int i = 0; i <8; i++){
        BuffMatrix.put(emptySample);  //fill up buffer with empty samples so matrix starts off clear
    }
    for(int i = 0; i<8; i++)    //set default values upon startup
    {    
        CS = 0;       
        MatrixCMD.write(0); //write to right side
        MatrixCMD.write(0); //write to left side
        MatrixCMD.write(i); //row select
        CS = 1; 
    }
}

void MatrixGraph::setConsumerContext(ConsumerContext *Cont){
    Context = Cont;
    limits = Context->getThresholds();
    sampleSelect = Context->getDisplayParam();
    //watcher = Context->getwatchdog();
}

void MatrixGraph::consumeSample(sample_t sample)
{
    if(BuffMatrix.isFull()){    //when matrix buffer is full, start popping off the buffer too
        BuffMatrix.get();       //prevent buffer from filling up
    }
    BuffMatrix.put(sample);     //get new sample
    DispVar();                  //Hack figure out better way to update this
    

}

void MatrixGraph::DispVar()
{
    /*
        Takes a the desired environment variable chosen by azure as a parameter,
        and quantises the last 8 samples stored in the buffer
    */
    unsigned int PercentageResult;
    unsigned short thresholddelta; 
    unsigned short ConvertedSamples[8];
    
    unsigned short selectedsample;
    unsigned short lowerthreshold;
    
    //update contextual variables
    Context->getThreshLock(); //acquire threshold mutex lock from the network class
    limits = Context->getThresholds();
    Context->releaseThreshLock(); //release lock
    
    Context->getParamLock();
    sampleSelect = Context->getDisplayParam();
    Context->releaseParamLock();
    
    
    sample_t sample;
    switch(sampleSelect){
        case LIGHT:
            lowerthreshold = limits.light[LOWER]; //set lower threshold
            thresholddelta = limits.light[UPPER] - lowerthreshold; //calculate difference between high and low threshold
            break;
        case TEMP:
            lowerthreshold = limits.temp[LOWER]; //set lower threshold
            thresholddelta = limits.temp[UPPER] - lowerthreshold; //calculate difference between high and low threshold
            break;
        case PRESS:
            lowerthreshold = limits.press[LOWER]; //set lower threshold
            thresholddelta = limits.press[UPPER] - lowerthreshold; //calculate difference between high and low threshold
            break;
        default:
            PrintQueue.call(printf, "Default SampleSelect SW1\n\r");
            break;
    };

    for (int h = 0; h<=BuffMatrix.getSize(); h++){
        sample = BuffMatrix.peek(h); //get next samples for each environment variable
        switch (sampleSelect){
            case TEMP:
                if(sample.temp > limits.temp[UPPER])
                {
                    selectedsample = limits.temp[UPPER]; //set to max value if overrun
                }else if(sample.temp < lowerthreshold)
                {
                    selectedsample = lowerthreshold; //set to minimum to show it has overrun (prevents negative values)
                }else{
                    selectedsample = sample.temp; //store specific environment variable sample
                }
                break;

            case PRESS:
                if(sample.press > limits.press[UPPER])
                {
                    selectedsample = limits.press[UPPER]; //set to max value if overrun
                }else if(sample.press < lowerthreshold)
                {
                    selectedsample = lowerthreshold; //set to minimum to show it has overrun (prevents negative values)
                }else{
                    selectedsample = sample.press; //store specific environment variable sample
                }
                break;

            case LIGHT:
                if(sample.light > limits.light[UPPER])
                {
                    selectedsample = limits.light[UPPER]; //set to max value if overrun
                }else if(sample.light < lowerthreshold)
                {
                    selectedsample = lowerthreshold; //set to minimum to show it has overrun (prevents negative values)
                }else{
                    selectedsample = sample.light; //store specific environment variable sample
                }
                //PrintQueue.call(printf, "RowNo: %d\tSNo:\t%d\t Sample: %d\t\r\n",h, sample.sampleNo, sample.light);

                break;
                
            default:
                PrintQueue.call(printf, "Default SampleSelect SW2\n\r");
                //display temp by default
                lowerthreshold = limits.temp[LOWER]; //set lower threshold
                thresholddelta = limits.temp[UPPER] - lowerthreshold; //calculate difference between high and low threshold
                if(sample.temp > limits.temp[UPPER])
                {
                    selectedsample = limits.temp[UPPER]; //set to max value if overrun
                }else if(sample.temp < lowerthreshold)
                {
                    selectedsample = 0; //set to minimum to show it has overrun (prevents negative values)
                }else{
                    selectedsample = sample.temp; //store specific environment variable sample
                }
                break;
        }
    

        if (thresholddelta > 0){ //check if the threshold delta makes sense

            //PrintQueue.call(printf, "Limits U : \t%d \t%d \t%d\r\n", (int)limits.light[UPPER],(int)limits.temp[UPPER],(int)limits.press[UPPER]);
            //PrintQueue.call(printf, "Sample %d: \t%d \t%d \t%d\tDelta: %d\r\n",h, sample.light, sample.temp, sample.press,thresholddelta);
            //PrintQueue.call(printf, "Limits L : \t%d \t%d \t%d\r\n", (int)limits.light[LOWER],(int)limits.temp[LOWER],(int)limits.press[LOWER]);
            //PrintQueue.call(printf, "SampleNo %d:\t%d\t%d\t%d\tDelta: %d\r\n",h,sample.light, sample.temp, sample.press,thresholddelta);

            //PrintQueue.call(printf, "Nice\r\n");
        }else{
            PrintQueue.call(printf, "THE THRESHOLDS ARE SCUFFED\r\n");  //print to error log ... when we've created an error log
            //the threshold delta is equal to or less than 0, so thresholds were set backwards or were the same
        }
        
        PercentageResult = ((selectedsample-lowerthreshold)*1000)/thresholddelta;//result of sample as a percentage of its max value
        ConvertedSamples[h] = (16*PercentageResult)/1000; //quantise the percentage to x/16 to find out how many of the 16 LEDs on the row must be lit 
         
    }
    if(BufferLock.trylock_for(5000ms)) //critical section since left and right bytes are accessed by multiple functions
    {
        //watcher.kick();
        for (int i = 0; i<8; i++)
        {
            //convert the number of LEDs that need to be turned on into a binary number than can be sent to the matrix
            //2^(numLEDs) - 1 
            //E.G 5 LEDs need to be lit, (2^5)-1 = 31 in binary or 11111
            output_16[i] = (1 << ConvertedSamples[i]) - 1; 

            //split the converted number into left and right bytes for each half of the LED matrix
            
        }
    }else{
        PrintQueue.call(printf, "Trylock failed\n\r");

    }
    BufferLock.unlock();


}

void MatrixGraph::GraphWrite() //write LED values to matrix
{
    int r = 0;  //row number
    uint8_t leftbyte[8];
    uint8_t rightbyte[8];
    while(true){
        
        if(BufferLock.trylock_for(5000ms)) //Take the bufferlock since right and leftbytes are modified in DataIn
        {
            //watcher.kick();
            for(int i = 0; i<8; i++)    //loop through each sample
            {
                leftbyte[i] = (output_16[i] & 0xFF);            //split data into left and right bytes
                rightbyte[i] = (output_16[i] >> 8) & 0xFF;
                CS = 0;
                
                MatrixCMD.write(rightbyte[i]); //write to right side
                MatrixCMD.write(leftbyte[i]); //write to left side
                MatrixCMD.write(8-i); //row select (write from top row down)
                CS = 1; 
                //WatchdogCheck.kickGraph = WatchdogCheck.ENABLED; //kicks the watchdog for the graph thread
                ThisThread::sleep_for(stroberate); //only needs to be strobed every 3ms, go do something else

            }
        }else{
            PrintQueue.call(printf, "Trylock failed \n\r");
        }  
        BufferLock.unlock();

    }
}

osThreadId_t MatrixGraph::getThreadID()
{
    //get thread ID in case signals are needed later
    return ThisThread::get_id();
}

void MatrixGraph::locktest()
{
    //Lock for testing forced deadlocks
    BufferLock.lock();
}