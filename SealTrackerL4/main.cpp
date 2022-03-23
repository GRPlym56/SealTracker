/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "platform/mbed_thread.h"
#include "nRF24L01P.h"
#include "MS5837.h"

#define TRANSFER_SIZE   4


nRF24L01P Comms(PB_5, PB_4, PB_3, PB_0, PB_1, PA_8);
//I2C MS58(PA_10, PA_9); //SDA, SCL
MS5837 PressSens(PA_10, PA_9); //SDA, SCL


void sendmsg(char msg[4]);
void sendsample(char sample[3][4]);

int main() {
 
// The nRF24L01+ supports transfers from 1 to 32 bytes, but Sparkfun's
//  "Nordic Serial Interface Board" (http://www.sparkfun.com/products/9019)
//  only handles 4 byte transfers in the ATMega code.

 
    char txData[TRANSFER_SIZE], rxData[TRANSFER_SIZE];
    int txDataCnt = 0;
    int rxDataCnt = 0;
 
    Comms.powerUp();
 
    // Display the (default) setup of the nRF24L01+ chip
    printf( "nRF24L01+ Frequency    : %d MHz\r\n",  Comms.getRfFrequency() );
    printf( "nRF24L01+ Output power : %d dBm\r\n",  Comms.getRfOutputPower() );
    printf( "nRF24L01+ Data Rate    : %d kbps\r\n", Comms.getAirDataRate() );
    printf( "nRF24L01+ TX Address   : 0x%010llX\r\n", Comms.getTxAddress() );
    printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", Comms.getRxAddress() );
 
    printf( "Type keys to test transfers:\r\n  (transfers are grouped into %d characters)\r\n", TRANSFER_SIZE );
 
    Comms.setTransferSize( TRANSFER_SIZE );
 
    //Comms.setReceiveMode();
    //2d arrays for sending samples 
    char testsample[3][4] = {
        {'C','1','2','3'},
        {'T','4','5','6'},
        {'P','7','8','9'}
    };
    Comms.setTransmitMode();   
    Comms.enable();

    /*
    char cmd[2];
    char PROM[7];
    const int addr7bit = 0x76;
    const int addr8bit = 0xEC;

    const char Reset[1] = {0x1E};
    
    char PROMRead[7] = {0xA0, 0xA2, 0xA4, 0xA6, 0xA8, 0xAA, 0xAC}; 
    char ADCresult[3];
    

    MS58.frequency(400000); //400kHz SCL

    MS58.write(addr7bit, Reset, 1); //reset
    //MS58.write(addr7bit, PROMRead, 2);
    //MS58.read(addr7bit, PROM, 2);
    //printf("PROM: %X", PROM);
   

    cmd[0] = 0x48;
    cmd[1] = 0x00;
    MS58.write(addr7bit, cmd, 1);

    cmd[0] = 0x00;
    MS58.write(addr7bit, cmd, 1);
    MS58.read(addr7bit, ADCresult, 3);

    printf("ADC: %X \n\r", ADCresult);
    */

    PressSens.MS5837Init();
    
    float temp, press;
    

    while(1) {
 
       PressSens.Barometer_MS5837();
       temp = PressSens.MS5837_Temperature(); 
       press = PressSens.MS5837_Pressure();
       printf("Temp: %f \n\r Press: %f\n\r", temp, press);
       ThisThread::sleep_for(500ms);
        
        
    }

}

void sendmsg(char msg[4])
{
    
    Comms.write( NRF24L01P_PIPE_P0, msg, 4);
    ThisThread::sleep_for(500ms);
    
}

void sendsample(char sample[3][4])
{
    for(int i = 0; i<3; i++)
    {
        sendmsg(sample[i]); //send sample data
        sendmsg("\n\r"); //new line
        ThisThread::sleep_for(50ms); //timing slack
    }
}