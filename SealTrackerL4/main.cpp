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


nRF24L01P Comms(PB_5, PB_4, PB_3, PA_4, PB_1, PA_8);
//I2C MS58(PA_10, PA_9); //SDA, SCL
MS5837 PressSens(PA_10, PA_9); //SDA, SCL




Thread SealSumbersion;

void sendmsg(char msg[]);
void SubmersionDetection(void);


int main() {

    /*
        MAKE SURE BUILD PROFILE IN MBED STUDIO IS SET TO DEVELOP, OTHERWISE THE BOARD WILL NOT DEEP SLEEP
        this reduces average current by approx 6mA
    */
   
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
    if(sleep_manager_can_deep_sleep()) //check deep sleep
    {
        printf("deep sleep available \n\r");
    }
    SealSumbersion.start(SubmersionDetection);
    sendmsg("testing this is a long string!\n");
    
    while(1) 
    {

        ThisThread::sleep_for(100s);
        
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
/*TODO
-Sort out power distribution issue
-Get the NRF to power down when not in use (16mA oof)
*/



void SubmersionDetection()
{
    //i will use temperature as a demo variable for now since the device is not ready to be submerged
    while(1)
    {
        PressSens.Barometer_MS5837();
        //ThisThread::sleep_for(100ms);
        float temp = PressSens.MS5837_Temperature(); 
        float press = PressSens.MS5837_Pressure();
        char data[32];
        sprintf(data, "P: %f, T: %f\n", press, temp);
        if(temp > 10)
        {
            Comms.powerUp();
            ThisThread::sleep_for(5ms); //timing slack or something
            //sendmsg("The blubbery seal has surfaced\n");
            sendmsg(data);
            //powerdown sequence (saves ~7mA)
            Comms.powerDown();
            Comms.disable();
            
            
        }else 
        {
            //seal underwater, do nothing
        }
        
        ThisThread::sleep_for(4s);
        
    }
}



