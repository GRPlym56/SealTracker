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

#define AzureFlag (1UL << 9) //flag for azure to wait on

typedef struct  //template struct for SPI devices
{
    PinName MOSI;
    PinName MISO;
    PinName SCLK;
    PinName CS;
    PinName OE;

} SPIConfig_t ;


const SPIConfig_t SDpins{
    PB_5,
    PB_4,
    PB_3,
    PF_3
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
    PE_14,
    PE_13,
    PE_12,
    PE_11,
    PF_13, 
    PE_9

};


#endif
