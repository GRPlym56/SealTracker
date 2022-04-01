#include "mbed.h"
#include "nRF24L01P.h"
 

#define TRANSFER_SIZE   32
nRF24L01P my_nrf24l01p(PA_7, PA_6, PA_5, PE_11, PF_13, PE_9);    // mosi, miso, sck, csn, ce, irq
 
DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
 
int main() {
 
// The nRF24L01+ supports transfers from 1 to 32 bytes, but Sparkfun's
//  "Nordic Serial Interface Board" (http://www.sparkfun.com/products/9019)
//  only handles 4 byte transfers in the ATMega code.

 
    char txData[TRANSFER_SIZE], rxData[TRANSFER_SIZE];
    int txDataCnt = 0;
    int rxDataCnt = 0;
 
    my_nrf24l01p.powerUp();
 
    // Display the (default) setup of the nRF24L01+ chip
    printf( "nRF24L01+ Frequency    : %d MHz\r\n",  my_nrf24l01p.getRfFrequency() );
    printf( "nRF24L01+ Output power : %d dBm\r\n",  my_nrf24l01p.getRfOutputPower() );
    printf( "nRF24L01+ Data Rate    : %d kbps\r\n", my_nrf24l01p.getAirDataRate() );
    printf( "nRF24L01+ TX Address   : 0x%010llX\r\n", my_nrf24l01p.getTxAddress() );
    printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", my_nrf24l01p.getRxAddress() );
 
    printf( "Type keys to test transfers:\r\n  (transfers are grouped into %d characters)\r\n", TRANSFER_SIZE );
 
    my_nrf24l01p.setTransferSize( TRANSFER_SIZE );
 
    my_nrf24l01p.setReceiveMode();
    my_nrf24l01p.enable();

    while (1)
    {
        if ( my_nrf24l01p.readable() ) {
 
            // ...read the data into the receive buffer
            rxDataCnt = my_nrf24l01p.read( NRF24L01P_PIPE_P0, rxData, sizeof( rxData ) );
 
            // Display the receive buffer contents via the host serial link
            for ( int i = 0; rxDataCnt > 0; rxDataCnt--, i++ ) {
 
                printf("%c", rxData[i] );
            }
 
            // Toggle LED2 (to help debug nRF24L01+ -> Host communication)
            myled2 = !myled2;
        }

        
    }

}