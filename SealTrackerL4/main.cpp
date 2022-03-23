/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "platform/mbed_thread.h"
#include "nRF24L01P.h"
#include "MS5837.h"
#include "string.h"

#define TRANSFER_SIZE   32


nRF24L01P Comms(PB_5, PB_4, PB_3, PB_0, PB_1, PA_8);
//I2C MS58(PA_10, PA_9); //SDA, SCL
MS5837 PressSens(PA_10, PA_9); //SDA, SCL


Thread SealSumbersion;

void sendmsg(char msg[]);
void sendsample(char sample[3][4]);
void SubmersionDetection(void);

int main() {
 
// The nRF24L01+ supports transfers from 1 to 32 bytes, but Sparkfun's
//  "Nordic Serial Interface Board" (http://www.sparkfun.com/products/9019)
//  only handles 4 byte transfers in the ATMega code.

 
    char txData[TRANSFER_SIZE], rxData[TRANSFER_SIZE];
    int txDataCnt = 0;
    int rxDataCnt = 0;
 
    Comms.powerUp(); //this seems to help sometimes
    Comms.powerDown();
    Comms.powerUp();
 
    // Display the (default) setup of the nRF24L01+ chip
    printf( "nRF24L01+ Frequency    : %d MHz\r\n",  Comms.getRfFrequency() );
    printf( "nRF24L01+ Output power : %d dBm\r\n",  Comms.getRfOutputPower() );
    printf( "nRF24L01+ Data Rate    : %d kbps\r\n", Comms.getAirDataRate() );
    printf( "nRF24L01+ TX Address   : 0x%010llX\r\n", Comms.getTxAddress() );
    printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", Comms.getRxAddress() );
 
    printf( "Type keys to test transfers:\r\n  (transfers are grouped into %d characters)\r\n", TRANSFER_SIZE );
 
    Comms.setTransferSize( TRANSFER_SIZE );
 
    
    Comms.setTransmitMode();   
    Comms.enable();

    
   

    PressSens.MS5837Init();
    
    float temp, press;
    
    //SealSumbersion.start(SubmersionDetection);

    while(1) {
 
        /*
       PressSens.Barometer_MS5837();
       temp = PressSens.MS5837_Temperature(); 
       press = PressSens.MS5837_Pressure();
       printf("Temp: %f \n\r Press: %f\n\r", temp, press);
       ThisThread::sleep_for(500ms);
       */
        sendmsg("testing\n\r");
        
        ThisThread::sleep_for(1000ms);
        
    }

}

void sendmsg(char msg[])
{
    
    unsigned int length = strlen(msg);
    if(length <= 32)
    {
        Comms.write( NRF24L01P_PIPE_P0, msg, 32);
        ThisThread::sleep_for(25ms);
    }
    else
    {
        printf("Message too long \n\r");
    }
    
   
    
}

void sendsample(char sample[3][4])
{
    for(int i = 0; i<3; i++)
    {
        sendmsg(sample[i]); //send sample data
        sendmsg("  \n"); //new line
        ThisThread::sleep_for(50ms); //timing slack
    }
}

void SubmersionDetection()
{
    //i will use temperature as a demo variable for now since the device is not ready to be submerged
    while(1)
    {
        PressSens.Barometer_MS5837();
        float temp = PressSens.MS5837_Temperature(); 
        float press = PressSens.MS5837_Pressure();
        if(temp > 19.5)
        {
            sendmsg("Seals are blubbery");
            sendmsg("\n");
        }else 
        {
            //seal underwater, do nothing
        }
        ThisThread::sleep_for(4s);
        
    }
}