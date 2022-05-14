#include "CommsWrapper.hpp"

CommsWrapper::CommsWrapper(NRFPINS Pins): Comms(Pins.mosi, Pins.miso, Pins.sck, Pins.csn, Pins.ce, Pins.irq)
{
    //init sequence
    //Comms.powerUp();
    
    
    //PrintQueue.call(printf, "Comms powerup\n\r"); 
}

void CommsWrapper::InitSendNode()
{
    /*
    PrintQueue.call(printf, "nRF24L01+ Frequency    : %d MHz\r\n",  Comms.getRfFrequency() );
    PrintQueue.call(printf, "nRF24L01+ Output power : %d dBm\r\n",  Comms.getRfOutputPower() );
    PrintQueue.call(printf, "nRF24L01+ Data Rate    : %d kbps\r\n", Comms.getAirDataRate() );
    PrintQueue.call(printf, "nRF24L01+ TX Address   : 0x%010llX\r\n", Comms.getTxAddress() );
    PrintQueue.call(printf, "nRF24L01+ RX Address   : 0x%010llX\r\n", Comms.getRxAddress() );
    */
    Comms.setAirDataRate(DATARATE);
    Comms.setTransferSize(32); //maximum message size 
    Comms.setTransmitMode(); 
    Comms.enable(); //go!
    PrintQueue.call(printf, ("Send mode set\n\r"));
    // Display the setup of the nRF24L01+ chip
    

   
}

void CommsWrapper::InitReceiveNode()
{
    Comms.setAirDataRate(DATARATE);
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
        Comms.enable();
        PrintQueue.call(printf, "Sending message:\n\r");
       
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
void CommsWrapper::SendmsgNoPwrCntrl(char msg[]) 
{
    //unsigned int length = strlen(msg);
    if(strlen(msg) <= 32)
    {   
        Comms.write( NRF24L01P_PIPE_P0, msg, TRANSFER_SIZE); //send message
        ThisThread::sleep_for(25ms);
    }
    else
    {
        PrintQueue.call(printf, "Message too long \n\r");
    }
}

void CommsWrapper::RequestTime()
{
    bool done = false;
    PrintQueue.call(printf, "Requesting time from 429\n\r");
    Sendmsg("Time Please"); //request time from the F429
    InitReceiveNode();
    
    do
    {
        if(Comms.readable())
        {
            rxDataCnt = Comms.read( NRF24L01P_PIPE_P0, rxData, sizeof( rxData ) );
                
            // Display the receive buffer contents via the host serial link
            for ( int i = 0; rxDataCnt > 0; rxDataCnt--, i++ ) {

                char a = rxData[i];
                    
            }
            PrintQueue.call(printf, "message received\n\r");
            done = true;
            int time =  stoi(rxData); //convert receive string to integer 
            set_time(time); //set up RTC
        }


    }while(done == false);


}

void CommsWrapper::On()
{
    Comms.powerUp();
    Comms.enable();
}

void CommsWrapper::Off()
{
    Comms.powerDown();
    Comms.disable();
}

