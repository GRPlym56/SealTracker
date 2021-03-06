#include <stdlib.h>
#include "MS5837.h"


/*
 * Sensor operating function according data sheet
 */

void MS5837::MS5837Init(void)
{
    MS5837Reset();
    MS5837ReadProm();
    return;
}

/* Send soft reset to the sensor */
void MS5837::MS5837Reset(void)
{
    /* transmit out 1 byte reset command */
    ms5837_tx_data[0] = ms5837_reset;
    if ( i2c.write( device_address,  ms5837_tx_data, 1 ) );
    printf("send soft reset\n");
    ThisThread::sleep_for(20ms);
}

/* read the sensor calibration data from rom */
void MS5837::MS5837ReadProm(void)
{
    uint8_t i,j;
    for (i=0; i<8; i++) {
        j = i;
        ms5837_tx_data[0] = ms5837_PROMread + (j<<1);
        if ( i2c.write( device_address,  ms5837_tx_data, 1 ) );
        if ( i2c.read( device_address,  ms5837_rx_data, 2 ) );
        C[i]   = ms5837_rx_data[1] + (ms5837_rx_data[0]<<8);
    }
}

/* Start the sensor pressure conversion */
void MS5837::MS5837ConvertD1(void)
{
    ms5837_tx_data[0] = ms5837_convD1;
    if ( i2c.write( device_address,  ms5837_tx_data, 1 ) );
}

/* Start the sensor temperature conversion */
void MS5837:: MS5837ConvertD2(void)
{
    ms5837_tx_data[0] = ms5837_convD2;
    if ( i2c.write( device_address,  ms5837_tx_data, 1 ) );
}

/* Read the previous started conversion results */
int32_t MS5837::MS5837ReadADC(void)
{
    int32_t adc;
    ThisThread::sleep_for(150ms);
    ms5837_tx_data[0] = ms5837_ADCread;
    if ( i2c.write( device_address,  ms5837_tx_data, 1 ) );
    if ( i2c.read( device_address,  ms5837_rx_data, 3 ) );
    adc = ms5837_rx_data[2] + (ms5837_rx_data[1]<<8) + (ms5837_rx_data[0]<<16);
    //printf("ADC value: %x\n", adc);
    return (adc);
}

/* return the results */
float MS5837::MS5837_Pressure (void)
{
    if(SensorLock.trylock_for(5s))
    {
        float pressure = P_MS5837;
        SensorLock.unlock();
        return pressure;
    }else 
    {
        return 0.0f;
    }
}
float MS5837::MS5837_Temperature (void)
{
    if(SensorLock.trylock_for(5s))
    {
        float temperature = T_MS5837;
        SensorLock.unlock();
        return temperature;
    }else 
    {
        return 0.0f;
    }
}

/* Sensor reading and calculation procedure */
void MS5837::Barometer_MS5837(void)
{
    if(SensorLock.trylock_for(5s))
    {
        int32_t dT, temp;
        int64_t OFF, SENS, press;

        //no need to do this everytime!
        
        
        MS5837ConvertD1();             // start pressure conversion
        D1 = MS5837ReadADC();        // read the pressure value
        MS5837ConvertD2();             // start temperature conversion
        D2 = MS5837ReadADC();         // read the temperature value
        //printf("D1 = %d\n", D1);
        /* calculation according MS5837-01BA data sheet DA5837-01BA_006 */
        dT       = D2 - (C[5]* 256);
        OFF      = (int64_t)C[2] * (1<<16) + ((int64_t)dT * (int64_t)C[4]) / (1<<7);
        SENS     = (int64_t)C[1] * (1<<15) + ((int64_t)dT * (int64_t)C[3]) / (1<<8);

        temp     = 2000 + (dT * C[6]) / (1<<23);
        T_MS5837 = (float) temp / 100.0f;                 // result of temperature in deg C in this var
        press    = (((int64_t)D1 * SENS) / (1<<21) - OFF) / (1<<13);
        P_MS5837 = (float) press / 10.0f;                 // result of pressure in mBar in this var
        
        if (P_MS5837 < 900 || P_MS5837 > 30000) {
            MS5837Reset();                 // reset the sensor
            MS5837ReadProm();             // read the calibration values
        }
        SensorLock.unlock();
    }else 
    {
        PrintQueue.call(printf, "SensorLock trylock failed in barometer \n\r");
    }
}

float MS5837::depth(void) {
    return (P_MS5837/100)*1.019716;
}