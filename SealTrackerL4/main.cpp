/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "platform/mbed_thread.h"
#include "nRF24L01P.h"


Serial pc(USBTX, USBRX); // tx, rx

nRF24L01P Comms(PB_5, PB_4, PB_3, PB_0, PB_1, PA_8);


void sendmsg(char msg[4]);
void sendsample(char sample[3][4]);

int main() {
 
// The nRF24L01+ supports transfers from 1 to 32 bytes, but Sparkfun's
//  "Nordic Serial Interface Board" (http://www.sparkfun.com/products/9019)
//  only handles 4 byte transfers in the ATMega code.
#define TRANSFER_SIZE   4
 
    char txData[TRANSFER_SIZE], rxData[TRANSFER_SIZE];
    int txDataCnt = 0;
    int rxDataCnt = 0;
 
    Comms.powerUp();
 
    // Display the (default) setup of the nRF24L01+ chip
    pc.printf( "nRF24L01+ Frequency    : %d MHz\r\n",  Comms.getRfFrequency() );
    pc.printf( "nRF24L01+ Output power : %d dBm\r\n",  Comms.getRfOutputPower() );
    pc.printf( "nRF24L01+ Data Rate    : %d kbps\r\n", Comms.getAirDataRate() );
    pc.printf( "nRF24L01+ TX Address   : 0x%010llX\r\n", Comms.getTxAddress() );
    pc.printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", Comms.getRxAddress() );
 
    pc.printf( "Type keys to test transfers:\r\n  (transfers are grouped into %d characters)\r\n", TRANSFER_SIZE );
 
    Comms.setTransferSize( TRANSFER_SIZE );
 
    //Comms.setReceiveMode();
    //2d arrays for sending samples 
    char testsample[3][4] = {
        {'C','1','2','3'},
        {'T','4','5','6'},
        {'P','7','8','9'}};
    Comms.enable();
 
    while (1) {
 
        /*
        // If we've received anything over the host serial link...
        if ( pc.readable() ) {
 
            // ...add it to the transmit buffer
            txData[txDataCnt++] = pc.getc();
 
            // If the transmit buffer is full
            if ( txDataCnt >= sizeof( txData ) ) {
 
                // Send the transmitbuffer via the nRF24L01+
                Comms.write( NRF24L01P_PIPE_P0, "test", txDataCnt );
 
                txDataCnt = 0;
            }
 
            // Toggle LED1 (to help debug Host -> nRF24L01+ communication)
            //myled1 = !myled1;
        }
 
        // If we've received anything in the nRF24L01+...
        if ( Comms.readable() ) {
 
            // ...read the data into the receive buffer
            rxDataCnt = Comms.read( NRF24L01P_PIPE_P0, rxData, sizeof( rxData ) );
 
            // Display the receive buffer contents via the host serial link
            for ( int i = 0; rxDataCnt > 0; rxDataCnt--, i++ ) {
 
                pc.putc( rxData[i] );
            }
 
            // Toggle LED2 (to help debug nRF24L01+ -> Host communication)
            //myled2 = !myled2;
        }
    
    */
        //sendsample(testsample);
    
        ThisThread::sleep_for(1000);
        
    }

}

void sendmsg(char msg[4])
{
    
    Comms.write( NRF24L01P_PIPE_P0, msg, 4);
    
}

void sendsample(char sample[3][4])
{
    for(int i = 0; i<3; i++)
    {
        sendmsg(sample[i]); //send sample data
        sendmsg("\n\r"); //new line
        ThisThread::sleep_for(50); //timing slack
    }
}