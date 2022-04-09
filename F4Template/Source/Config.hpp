/*
ELEC-351 Group E
Author: Jacob Howell, Guy Ringshaw, Ethan Brian
December-January 2022
*/
#ifndef _CONFIG_H__
#define _CONFIG_H__
#include "mbed.h"
//#include "uop_msb.h"


typedef struct  //template struct for SPI devices
{
    PinName MOSI;
    PinName MISO;
    PinName SCLK;
    PinName CS;
    PinName OE;

} SPIConfig_t ;


const SPIConfig_t BMP280_Pins{
    PB_5,
    PB_4,
    PB_3,
    PB_2 
};

const SPIConfig_t MatrixPins
{
    PC_12, //MOSI
    PC_11, //dummy MISO (not needed, dunno bro (IOE))
    PC_10, //SCLK
    PB_6,  //CS
    PB_12, //OE
};

const SPIConfig_t SDpins{
    PB_5,
    PB_4,
    PB_3,
    PF_3
};

enum THRESHOLDS{LOWER = 0, UPPER};

typedef struct {
            unsigned short light[2];
            unsigned short temp[2];
            unsigned short press[2];
}Thresholds_t;


const Thresholds_t initLimits{
    {20,60},        //Light
    {15,30},        //Temp
    {950, 1150}     //Press
};


typedef struct{
            bool valid;
            unsigned short sampleNo;
            unsigned short temp;
            unsigned short press;
            unsigned short light;
            char date[11];
            char time[9];
}sample_t;

typedef struct 
{
    float temp;
    float light;

} sealsample_t;


typedef struct 
{
    PinName mosi; 
    PinName miso; 
    PinName sck; 
    PinName csn;
    PinName ce;
    PinName irq;
} NRFPINS;

const NRFPINS RFPINS
{
    PE_14,
    PE_13,
    PE_12,
    PE_11,
    PF_13, 
    PE_9

};

const PinName BLUEBUTTON = PC_13;
const PinName WARNINGLIGHT = PC_3;

// typedef struct {char message[128];} errorLog_t;

#endif
