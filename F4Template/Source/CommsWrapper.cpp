#include "CommsWrapper.hpp"

CommsWrapper::CommsWrapper(NRFPINS Pins): Comms(Pins.mosi, Pins.miso, Pins.sck, Pins.csn, Pins.ce, Pins.irq), CommsLED(PB_7)
{
    //init sequence
    Comms.powerUp();
   printf("Comms powerup\n\r"); 
}

void CommsWrapper::InitSendNode()
{
    Comms.setTransferSize(32); //maximum message size 
    Comms.setTransmitMode(); //this is the receiver
    Comms.enable(); //go!
   
}

void CommsWrapper::InitReceiveNode()
{
    Comms.setTransferSize(32); //maximum message size 
    Comms.setReceiveMode(); //this is the receiver
    Comms.enable(); //go!
    PrintQueue.call(printf, ("Receive mode set\n\r"));
}

/*TODO
fix printing issue,
get the samples sent to azure
get the samples on the SD card
*/
void CommsWrapper::ReceiveData()
{
    while (true) {
        
        
        if ( Comms.readable() ) {
            PrintQueue.call(printf, ("Data ready\n\r"));
 
            // ...read the data into the receive buffer
            rxDataCnt = Comms.read( NRF24L01P_PIPE_P0, rxData, sizeof( rxData ) );
            
            // Display the receive buffer contents via the host serial link
            for ( int i = 0; rxDataCnt > 0; rxDataCnt--, i++ ) {

                char a = rxData[i];
                printf("%c", a);
                
            }
 
            // Toggle LED2 (to help debug nRF24L01+ -> Host communication)
            
            CommsLED = !CommsLED;
        }
    }
}
