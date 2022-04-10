#include "CommsWrapper.hpp"

CommsWrapper::CommsWrapper(NRFPINS Pins, DigitalOut CommsLED): Comms(Pins.mosi, Pins.miso, Pins.sck, Pins.csn, Pins.ce, Pins.irq), LED(CommsLED)
{
    //init sequence
    Comms.powerUp();
    
    printf("Comms powerup\n\r"); 
}

void CommsWrapper::InitSendNode()
{
    Comms.setTransferSize(32); //maximum message size 
    Comms.setTransmitMode(); 
    Comms.enable(); //go!
   
}

void CommsWrapper::InitReceiveNode()
{
    Comms.setAirDataRate(NRF24L01P_DATARATE_250_KBPS);
    Comms.setTransferSize(32); //maximum message size 
    Comms.setReceiveMode(); 
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
        
        
        
        if ( Comms.readable() ) { //rapid polling pepega
            //PrintQueue.call(printf, ("Data ready\n\r"));
 
            // ...read the data into the receive buffer
            rxDataCnt = Comms.read( NRF24L01P_PIPE_P0, rxData, sizeof( rxData ) );
            
            // Display the receive buffer contents via the host serial link
            for ( int i = 0; rxDataCnt > 0; rxDataCnt--, i++ ) {

                char a = rxData[i];
                //printf("%c", a);
                
            }
           
            PrintQueue.call(printf, "%s", rxData);
            Decode();
            // Toggle LED2 (to help debug nRF24L01+ -> Host communication)
            
            LED = !LED;
        }
    }
}

void CommsWrapper::Decode()
{
    std::string s; //string for holding receive data
    std::string delimiter = "|"; //delimiter for splitting strings
    //std::string token;
    
    s = rxData;
    size_t pos = 0;
    std::string token;
    int count = 0;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        //std::cout << token << std::endl;
        if(count == 0)
        {
            rxDataFormatted.pressure = token;
        }else if(count == 1)
        {
            rxDataFormatted.temperature = token;
        }
        s.erase(0, pos + delimiter.length());
        count++;
    }
    SampleBuffer.Put(rxDataFormatted);
    
}
