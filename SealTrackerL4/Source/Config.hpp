/*
    Written by Jacob Howell, Ethan Brian and Guy Ringshaw for ELEC 351 2021-2022

    Modified by Guy Ringshaw for Proj324 2022
*/
#ifndef _CONFIG_H__
#define _CONFIG_H__
#include "mbed.h"
#include <iostream>
#include <string>


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
    PB_5, PB_4, PB_3, PA_4, PB_1, PA_8

};

#endif
