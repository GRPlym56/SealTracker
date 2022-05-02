/*
ELEC-351 Group E
Author: Jacob Howell, Guy Ringshaw, Ethan Brian
December-January 2022
*/
#ifndef _CONFIG_H__
#define _CONFIG_H__
#include "mbed.h"
#include <iostream>
#include <string>

//#include "uop_msb.h"


typedef struct  //template struct for SPI devices
{
    PinName MOSI;
    PinName MISO;
    PinName SCLK;
    PinName CS;
    PinName OE;

} SPIConfig_t ;



const SPIConfig_t SDpins{
    PA_12,
    PA_6,
    PA_5,
    PB_0
};

typedef struct 
{
    std::string pressure;
    std::string temperature;
    std::string time;

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
    PB_5, PB_4, PB_3, PA_4, PB_1, PA_8

};


// typedef struct {char message[128];} errorLog_t;

#endif
