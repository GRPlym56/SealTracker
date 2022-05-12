/*
    Written by Jacob Howell, Ethan Brian and Guy Ringshaw for ELEC 351 2021-2022

    Modified by Guy Ringshaw for Proj324 2022
*/
#ifndef _CONFIG_H__
#define _CONFIG_H__
#include "mbed.h"
#include <iostream>
#include <string>


enum class sealstate_t
{
    RESTING = 0,
    CRUISING = 1,
    DIVING = 2,
    ASCENDING = 3,
    SURFACE = 4,
    UNKNOWN = 5

};

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
    float pressure;
    float temperature;
    std::string time;
    sealstate_t state;

} sealsampleL4_t;

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
    PB_5_ALT0, PB_4_ALT0, PB_3_ALT0, PA_4_ALT0, PB_1, PA_8, 

};



#endif
