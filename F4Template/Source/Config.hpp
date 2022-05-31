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


#define AzureFlag (1UL << 1) //flag for azure to wait on

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
    PB_5,
    PB_4,
    PB_3,
    PF_3
};

typedef struct 
{
    PinName Pin;
    PinMode Mode;
} ButtonConfig_t; 

const ButtonConfig_t ButtonPins
{
    PG_0,
    PullUp
};

typedef struct 
{
    float pressure;
    float temperature;
    std::string time;
    std::string state;

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
