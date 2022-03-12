//#include "mbed.h"
#include "BMP280_SPI_Handler.hpp"
#include <cstdint>

BMP280_SPI_Handler::BMP280_SPI_Handler(SPIConfig_t Pins):
                    spi_device(Pins.MOSI,Pins.MISO, Pins.SCLK), 
                    cs(Pins.CS)
{
    init();
}
BMP280_SPI_Handler::~BMP280_SPI_Handler()
{
}

void BMP280_SPI_Handler::init(){
    
    unsigned short osrs_t = 0b01;
    unsigned short osrs_p = 0b01;
    unsigned short mode = 0b01;
    uint8_t data[18];
    cs = 1;         //set chip select high by default
    spi_device.format(8,0); //8-bit mode zero
    spi_device.frequency(1000000);  //set SCLK to 1 MHz

    cs = 0;
    spi_device.write(ID);         //read chip ID
    data[0] = spi_device.write(0);   //read chip ID to check if its running
    cs = 1;

    cs = 0;
    spi_device.write(CTRL_MEAS); //set the data acquisition options of the device.
    spi_device.write((OSx1<<5)|(OSx1<<2)|(FORCED)); //temp oversampling x1, press oversampling x1, FORCED mode
    cs = 1;

    //TODO check if config cmd is needed
    //TODO check if wait is required/allowed
    wait_us(1000000);

    cs = 0;
    spi_device.write(DIG_T);    //read tempreture trimming parameters
    for (int i = 0; i<6; i++){
        data[i] = spi_device.write(0); //get next parameter
    }
    cs = 1;
    //data is 16bit values, but stored in 8 bit words. concatonate result
    dig_T1 = (data[1] << 8) | data[0];
    dig_T2 = (data[3] << 8) | data[2];
    dig_T3 = (data[5] << 8) | data[4];

    cs = 0;
    spi_device.write(DIG_P);
    for (int i = 0; i<18; i++){
        data[i] = spi_device.write(0); //get next parameter
    }
    cs = 1;
    dig_P1 = (data[1] << 8) | data[0];
    dig_P2 = (data[3] << 8) | data[2];
    dig_P3 = (data[5] << 8) | data[4];
    dig_P4 = (data[7] << 8) | data[6];
    dig_P5 = (data[9] << 8) | data[8];
    dig_P6 = (data[11] << 8) | data[10];
    dig_P7 = (data[13] << 8) | data[12];
    dig_P8 = (data[15] << 8) | data[14];
    dig_P9 = (data[17] << 8) | data[16];
}

int32_t BMP280_SPI_Handler::getTempreture(){
    enum bits{MSB = 0, LSB, XLSB};
    int32_t raw_data, var1, var2, tempreture;
    uint8_t data[3];    //create space for bit to be stored

    //get sample
    cs = 0;
    spi_device.write(TEMP_MSB); //read contense at first temp register
    for (int i = 0; i <=2; i++){
        data[i] = spi_device.write(0); //auto incremented register returns LSB and XLSB
    }
    cs = 1;
    raw_data = (data[MSB]<<12) | (data[LSB]<<4) | (data[XLSB]>>4); //concatonate data into one 20bit binary num

    //calibrate raw data
    var1 = ((((raw_data >> 3) - (dig_T1 << 1))) * dig_T2) >> 11;
    var2 = (((((raw_data >> 4) - dig_T1) * ((raw_data >> 4) - dig_T1)) >> 12) * dig_T3) >> 14;

    t_fine = var1 + var2;
    tempreture = (t_fine * 5 + 128) >> 8;
    return tempreture;
}

int32_t BMP280_SPI_Handler::getPressure(){
    int32_t raw_data, pressure, var1, var2;
    uint8_t data[3];
    enum bits{MSB = 0, LSB, XLSB};

    //get sample
    cs = 0;
    spi_device.write(PRESS_MSB); //read contense at first pressure register
    for (int i = 0; i <=2; i++){
        data[i] = spi_device.write(0); //auto incremented register returns LSB and XLSB
    }
    cs = 1;
    raw_data = (data[MSB]<<12) | (data[LSB]<<4) | (data[XLSB]>>4); //concatonate data into one 20bit binary num


    //calibrate raw data
    var1 = (t_fine>>1) - 64000; 
    var2 = (((var1>>2) * (var1>>2)) >> 11 ) * (dig_P6); 
    var2 = var2 + (((var1*dig_P5))<<1); 
    var2 = (var2>>2)+(dig_P4<<16); 
    var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((dig_P2 * var1)>>1))>>18; 
    var1 =((((32768+var1))*(dig_P1))>>15); 
    if (var1 == 0) 
    { 
    return 0; // avoid exception caused by division by zero 
    } 
    pressure = ((((1048576)-raw_data)-(var2>>12)))*3125; 
    if (pressure < 0x80000000)  
    { 
    pressure = (pressure << 1) / (var1); 
    }  
    else  
    { 
    pressure = (pressure /var1) * 2; 
    } 
    var1 = ((dig_P9) * ((((pressure>>3) * (pressure>>3))>>13)))>>12; 
    var2 = ((pressure>>2) * (dig_P8))>>13; 
    pressure = (pressure + ((var1 + var2 + dig_P7) >> 4)); 
    return pressure;
}
