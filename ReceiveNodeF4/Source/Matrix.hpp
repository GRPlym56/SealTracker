/*
ELEC-351 Group E
Author: Guy Ringshaw
December-January 2022
*/
#ifndef __MATRIX_H__
#define __MATRIX_H__
#include "mbed.h"
#include "SPI.h"
#include "Config.hpp"
#include "uop_msb.h"
#include "sampleReceiver.hpp"
#include "cbuff.hpp"
#include <cstdint>
extern EventQueue PrintQueue;

using namespace uop_msb;

class MatrixGraph : public SampleConsumer //inherit receiver class
{

    public:

        MatrixGraph(SPIConfig_t MatrixPins); //constructor with SPI pins
      
        void DispVar(); //takes current sample for a given environment variable and quantises it 

        void GraphWrite(); //strobes the matrix with quantised data

        osThreadId_t getThreadID(); //gets current thread ID

        void locktest(); //get lock for testing deadlocks
        void setConsumerContext(ConsumerContext* Context) override; //overrided function for getting thresholds and network lock
        void consumeSample(sample_t sample) override; //overrided function for getting sample data
        //void getwachdog() override;
        Circular_Buff <sample_t> BuffMatrix; //buffer for the matrix
        
    private:

    enum SAMPLE_TYPES{LIGHT = 0, TEMP, PRESS}; //enum for environment variables as parameters
    
    uint16_t output_16[8];//16 bit output for matrix display

    SPI MatrixCMD; //SPI object for writing to the matrix
    DigitalOut CS; //SPI chip select
    DigitalOut OE; //SPI object enable, unsused, but required for SPI to work somehow

    Mutex BufferLock; //mutex lock to prevent left and right bytes from being written to and read from simultaneously
       
    chrono::milliseconds stroberate = 3ms; //slowest strobe rate before the matrix starts to look too flickery
    ConsumerContext* Context; 
    Thresholds_t limits;
    unsigned int sampleSelect = LIGHT;
   
};

#endif