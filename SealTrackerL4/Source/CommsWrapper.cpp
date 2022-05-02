#include "CommsWrapper.hpp"

CommsWrapper::CommsWrapper(NRFPINS Pins): Comms(Pins.mosi, Pins.miso, Pins.sck, Pins.csn, Pins.ce, Pins.irq)
{
    //init sequence
    Comms.powerUp();
    
    PrintQueue.call(printf, "Comms powerup\n\r"); 
}

void CommsWrapper::InitSendNode()
{
    Comms.setAirDataRate(NRF24L01P_DATARATE_250_KBPS);
    Comms.setTransferSize(TRANSFER_SIZE); //maximum message size 
    Comms.setTransmitMode(); 
    Comms.enable(); //go!

    // Display the setup of the nRF24L01+ chip
    PrintQueue.call(printf, "nRF24L01+ Frequency    : %d MHz\r\n",  Comms.getRfFrequency() );
    PrintQueue.call(printf, "nRF24L01+ Output power : %d dBm\r\n",  Comms.getRfOutputPower() );
    PrintQueue.call(printf, "nRF24L01+ Data Rate    : %d kbps\r\n", Comms.getAirDataRate() );
    PrintQueue.call(printf, "nRF24L01+ TX Address   : 0x%010llX\r\n", Comms.getTxAddress() );
    PrintQueue.call(printf, "nRF24L01+ RX Address   : 0x%010llX\r\n", Comms.getRxAddress() );
   
}

void CommsWrapper::InitReceiveNode()
{
    Comms.setAirDataRate(NRF24L01P_DATARATE_250_KBPS);
    Comms.setTransferSize(TRANSFER_SIZE); //maximum message size 
    Comms.setReceiveMode(); 
    Comms.enable(); //go!
    PrintQueue.call(printf, ("Receive mode set\n\r"));
}

void CommsWrapper::Sendmsg(char msg[]) //function for sending one message
{
    //unsigned int length = strlen(msg);
    if(strlen(msg) <= 32)
    {
        Comms.powerUp(); //reactivate comms
        PrintQueue.call(printf, "Sending message:\n\r");
        PrintQueue.call(printf, msg);
        Comms.write( NRF24L01P_PIPE_P0, msg, TRANSFER_SIZE); //send message
        ThisThread::sleep_for(25ms);
        //power down and disable comms after message sent to save power
        Comms.powerDown();
        Comms.disable();
    }
    else
    {
        PrintQueue.call(printf, "Message too long \n\r");
    }
}

//function for sending data when lots of data needs to be sent quickly, it is on the user to manage power once it is done
void CommsWrapper::DataDump(char msg[]) 
{
    //unsigned int length = strlen(msg);
    if(strlen(msg) <= 32)
    {   
        Comms.write( NRF24L01P_PIPE_P0, msg, TRANSFER_SIZE); //send message
        ThisThread::sleep_for(25ms); //this needs to be tuned

    }
    else
    {
        PrintQueue.call(printf, "Error: Message too long \n\r");
    }
}

void CommsWrapper::On()
{
    Comms.powerUp();
}

void CommsWrapper::Off()
{
    Comms.powerDown();
    Comms.disable();
}

