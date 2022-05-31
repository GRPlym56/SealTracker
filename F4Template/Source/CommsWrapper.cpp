#include "CommsWrapper.hpp"

CommsWrapper::CommsWrapper(NRFPINS Pins, DigitalOut CommsLED, CircBuff* SD, CircBuff* Net, ButtonConfig_t BPins): 
Comms(Pins.mosi, Pins.miso, Pins.sck, Pins.csn, Pins.ce, Pins.irq), LED(CommsLED), sdbuff(SD), netbuff(Net), SW1(BPins.Pin, BPins.Mode) 
{
    //init sequence
    Comms.powerUp();
    
    printf("Comms powerup\n\r"); 
}

void CommsWrapper::InitSendNode()
{
   

    Comms.setAirDataRate(DATARATE);
    Comms.setTransferSize(TRANSFER_SIZE); //maximum message size 
    Comms.setTransmitMode(); 
    Comms.enable(); //go!
    PrintQueue.call(printf, "nRF24L01+ Frequency    : %d MHz\r\n",  Comms.getRfFrequency() );
    PrintQueue.call(printf, "nRF24L01+ Output power : %d dBm\r\n",  Comms.getRfOutputPower() );
    PrintQueue.call(printf, "nRF24L01+ Data Rate    : %d kbps\r\n", Comms.getAirDataRate() );
    PrintQueue.call(printf, "nRF24L01+ TX Address   : 0x%010llX\r\n", Comms.getTxAddress() );
    PrintQueue.call(printf, "nRF24L01+ RX Address   : 0x%010llX\r\n", Comms.getRxAddress() );
    PrintQueue.call(printf, "Send mode set\n\r");
   
}

void CommsWrapper::InitReceiveNode()
{
    

    Comms.setAirDataRate(DATARATE);
    Comms.setTransferSize(32); //maximum message size 
    Comms.setReceiveMode(); 
    Comms.enable(); //go!
    PrintQueue.call(printf, "nRF24L01+ Frequency    : %d MHz\r\n",  Comms.getRfFrequency() );
    PrintQueue.call(printf, "nRF24L01+ Output power : %d dBm\r\n",  Comms.getRfOutputPower() );
    PrintQueue.call(printf, "nRF24L01+ Data Rate    : %d kbps\r\n", Comms.getAirDataRate() );
    PrintQueue.call(printf, "nRF24L01+ TX Address   : 0x%010llX\r\n", Comms.getTxAddress() );
    PrintQueue.call(printf, "nRF24L01+ RX Address   : 0x%010llX\r\n", Comms.getRxAddress() );
    PrintQueue.call(printf, "Receive mode set\n\r");
}

void CommsWrapper::Sendmsg(char msg[]) //function for sending one message
{
    //unsigned int length = strlen(msg);
    if(strlen(msg) <= 32)
    {
        Comms.powerUp(); //reactivate comms
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

void CommsWrapper::ReceiveData()
{
    
    while (true) {
        
        
        
        if ( Comms.readable() ) { // ugly rapid polling 
            //PrintQueue.call(printf, ("Data ready\n\r"));
 
            // ...read the data into the receive buffer
            rxDataCnt = Comms.read( NRF24L01P_PIPE_P0, rxData, sizeof( rxData ) );
            
            // Display the receive buffer contents via the host serial link
            for ( int i = 0; rxDataCnt > 0; rxDataCnt--, i++ ) {

                char a = rxData[i];
                //printf("%c", a);
                
            }
           
            PrintQueue.call(printf, "%s\n\r", rxData);
            
            // Toggle LED2 (to help debug nRF24L01+ -> Host communication)
            Decode();
            LED = !LED;
        }
    }
}

void CommsWrapper::Decode()
{
    sealsample_t newsample;
    std::string s; //string for holding receive data
    std::string delimiter = "|"; //delimiter for splitting strings
    //std::string token;
    
    s = rxData;
    size_t pos = 0;
    std::string token[4];
    unsigned short count = 0;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token[count] = s.substr(0, pos);
        //PrintQueue.call(printf, "format: %s\n\r", token[count].c_str());
        count++;
        s.erase(0, pos + delimiter.length());
    }
    //format and put newly acquired date on appropriate buffers
    newsample.pressure = stof(token[0]);
    newsample.temperature = stof(token[1]);
    newsample.time = token[2];
    newsample.state = stoi(token[3]);

    sdbuff->Put(newsample); 
    netbuff->Put(newsample);

    osSignalSet(AzureThread, AzureFlag); //there is now data to send to azure

}

void CommsWrapper::SetAzureThreadID(osThreadId_t threadID)
{
    AzureThread = threadID;
}

void CommsWrapper::WaitForRequest()
{
    bool done = false;
    InitReceiveNode();
    
    do{    
        //PrintQueue.call(printf, "Ready for time request\n\r");
        if( Comms.readable() ) { // ugly rapid polling
            //PrintQueue.call(printf, ("Data ready\n\r"));
    
            // ...read the data into the receive buffer
            rxDataCnt = Comms.read( NRF24L01P_PIPE_P0, rxData, sizeof( rxData ) );
                
            // Display the receive buffer contents via the host serial link
            for ( int i = 0; rxDataCnt > 0; rxDataCnt--, i++ ) {

                char a = rxData[i];
                //printf("%c", a);
                    
            }
            LED = !LED;
            PrintQueue.call(printf, "%s\n\r", rxData);
           
            if(strncmp(rxData, "Time Please", 11) == 0)
            {
                ThisThread::sleep_for(250ms); //give a generous time window for the L432 to change to receive mode
                PrintQueue.call(printf, "L432 requests time\n\r");
                InitSendNode();
                time_t seconds = time(NULL);
                char EpochTime[32];
                sprintf(EpochTime, "%d", seconds); //convert time to char array for sending
                ThisThread::sleep_for(1s); //give the L432 time to switch to receive mode
                for(int i = 0; i<3; i++)
                {
                    Sendmsg(EpochTime);
                }
                done = true; //exit loop

            }else{
                PrintQueue.call(printf, "unknown request\n\r");
            }
           
            
            
        }

    }while(!done && !SW1.read()); //allow setup to be skipped if the L432 RTC is already configured
}
