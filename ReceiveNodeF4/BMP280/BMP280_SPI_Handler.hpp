#ifndef BMP280_SPI_HANDLER_H__
#define BMP280_SPI_HANDLER_H__

#include "mbed.h"
#include <cstdint>
#include "Config.hpp"

class BMP280_SPI_Handler{
public:
    BMP280_SPI_Handler(SPIConfig_t BMP280Pins);
    ~BMP280_SPI_Handler();

    void init();
    int32_t getTempreture();
    int32_t getPressure();

private:
    SPI spi_device;
    DigitalInOut cs;
    enum BMP280_MODE{SLEEP = 0, FORCED, NORMAL = 0b11}; //modes of opperation
    enum BMP280_osrs {SKIPPED = 0, OSx1, OSx2, OSx4, OSx8, OSx16};  //over sampling rate for tempreture or pressure
    enum BMP280_REGISTERS{  //REG ADDR Referance from BMP280 datasheet, from chap 4.2 onwards
        DIG_T       = 0x88, //start address for tempreture trimming parameters
        DIG_P       = 0x8E, //start address for pressure trimming parameters

        ID          = 0xD0, //ID register contains chip id (which is 0x58 by default)
        RESET       = 0xE0, //RESET contains the soft reset word "reset". if the value 0xB6 is written, the device is reset using the complete power on reset procedure.

        STATUS      = 0xF3, //The “status” register contains two bits which indicate the status of the device. (see chap 4.3.3 table 19 for details)
        CTRL_MEAS,          //The “ctrl_meas” register sets the data acquisition options of the device.
        CONFIGURE,          //The “config” register sets the rate, filter and interface options of the device. 

        //The “press” register contains the raw pressure measurement output data up[19:0]
        PRESS_MSB   = 0xF7, //Contains the MSB part up[19:12]
        PRESS_LSB,          //Contains the LSB part up[11:4]
        PRESS_XLSB,         //Contains the XLSB part up[3:0]

        //The “temp” register contains the raw temperature measurement output data ut[19:0]
        TEMP_MSB    = 0xFA, //Contains the MSB part ut[19:12]
        TEMP_LSB,           //Contains the LSB part up[11:4]
        TEMP_XLSB           //Contains the XLSB part up[3:0]
    };

    //calibration data for BMP280
    //tempreture trimming parameters 
    unsigned short dig_T1;
    short dig_T2;
    short dig_T3;
    //pressure trimming parameters
    unsigned short dig_P1;
    short dig_P2;
    short dig_P3;
    short dig_P4;
    short dig_P5;
    short dig_P6;
    short dig_P7;
    short dig_P8;
    short dig_P9;

    int32_t t_fine;
};

#endif
