/*
    written by Guy Ringshaw 2022
*/

#include "SealSubmersion.hpp"
#include <chrono>


SealSubmersion::SealSubmersion(CircBuff* Buff, CommsWrapper* Communications, MS5837* PressSens): Buffer(Buff), NRF(Communications), Sensor(PressSens)
{
    
    

}

void SealSubmersion::SurfaceDetection()
{
    //main dive tracking algorithm
    //looking for opportunities to send data at the surface and monitoring seal dive behaviour while using as few samples as possible
    //depth values may look funny as they are tuned for testing at a centimetre scale

    UpdateDepth(); //measure depth in metres and produce a delta

    if(depth[NOW] < 0.05f) //5cm 
    {
        if(!Buffer->IsEmpty()){ //check if buffer is empty before turning comms on
            PrintQueue.call(printf, "Buffer not empty, let's send data\n\r");
            char message[32];
            
            switch(SEAL_STATE)
            {
                case sealstate_t::RESTING: //seal is resting, that lump of blubber is going anywhere anytime soon! send all buffered samples
                    
                    do 
                    {
                        sealsampleL4_t sample = Buffer->Get(); //get data off buffer
                        sprintf(message, "%4.1f|%2.1f|%s|%d|", sample.pressure, sample.temperature, sample.time.c_str(), sample.state); //format message
                        NRF->Sendmsg(message); //send message  
                    }while(!Buffer->IsEmpty()); //repeat until buffer is empty
                   
                    break;

                default: //seal has just surfaced, only send a moderate amount of samples
                    
                    for(int i = 0; i<20; i++)
                    {
                        if(!Buffer->IsEmpty()) //make sure buffer still has samples to send before proceeding, buffer may have less than 20 samples
                        {
                            sealsampleL4_t sample = Buffer->Get(); //get data off buffer
                            sprintf(message, "%4.1f|%2.1f|%s|%d|", sample.pressure, sample.temperature, sample.time.c_str(), sample.state); //format message
                            NRF->Sendmsg(message); //send message  
                        }else {
                            break;
                        }
                    }
                    
                    break;

            }
            
            
        }
        RestTimer.start();
        if(RestTimer.elapsed_time() >= 60s)
        {
            //seal is probably hauled out
            SetSealState(sealstate_t::RESTING);
            RestTimer.stop(); 

            delay = 5000; //if seal is hauled out then nothing exciting will happen anytime soon, slow down
        }else 
        {
            SetSealState(sealstate_t::SURFACE);
            delay = 5000; //reset delay as seal may be about to dive or haul
        }
        
            
    }else if(delta_depth > 0.05f ) //seal diving? -- in reality this would be more like ~1 metre
    {
        RestTimer.stop();
        RestTimer.reset();
        SetSealState(sealstate_t::DIVING);
        
        delay = 5000; //moderate day to keep track of the dive
        
    }else if(delta_depth < -0.025f) //seal ascending? -- in reality this would be more like ~1 metre
    {
        RestTimer.stop();
        RestTimer.reset();
        SetSealState(sealstate_t::ASCENDING);

        delay = 5000; //moderate delay, dont want to miss a surfacing
        
    }else //seal must be maintaining its course or has just dipped back in the water
    {
        RestTimer.stop();
        RestTimer.reset();
        SetSealState(sealstate_t::CRUISING);
        delay = (depth[NOW]*3000)/10; //the deeper the seal, the longer the delay is (example: 100m depth = (100*3000)/10 = 30000ms delay)
    }

    ThisThread::sleep_for(delay); //using deprecated functin because it's easy to do arithmetic on an integer
}

void SealSubmersion::UpdateDepth() //measures current pressure value and updates depth
{
    //update prior value
    depth[PREVIOUS] = depth[NOW]; 
    

    Sensor->Barometer_MS5837(); //update values
    Pressure = Sensor->MS5837_Pressure(); //get pressure value from sensor class
    Temperature = Sensor->MS5837_Temperature(); //get temperature value from sensor class
    
    depth[NOW] = (Pressure - pressure_offset)/100.72; //remove ambient surface pressure, divide by 100.72 to get depth in metres
    PrintQueue.call(printf, "Depth: %f \n\r", depth[NOW]);

    delta_depth = depth[NOW] - depth[PREVIOUS];
    


}

void SealSubmersion::GetAmbientDepth() //convert ambient pressure value to depth to calibrate depth tracking
{

    float Pressure;
    for(unsigned short i = 0; i<10; i++)
    {
        Sensor->Barometer_MS5837(); //measure
        Pressure += Sensor->MS5837_Pressure(); //sum of measurements
        ThisThread::sleep_for(100ms); //overly generous timing slack as I very much want this to work
    }

    pressure_offset = Pressure/10; //calc mean
    
    PrintQueue.call(printf, "Ambient pressure: %4.2f \n\r", pressure_offset); //report ambient pressure 
    
}

sealstate_t SealSubmersion::GetSealState() //returns current state of seal
{
   
    if(BlubberLock.trylock_for(5s)) //try mutex for 5s
    {
        sealstate_t state = SEAL_STATE; //copy state so lock can be released
        BlubberLock.unlock(); //release lock
        return state; 
    }else 
    {
        PrintQueue.call(printf, "BlubberLock trylock failed in GetSealState \n\r");
        return sealstate_t::UNKNOWN; //error case
    }

}

void SealSubmersion::SetSealState(sealstate_t newstate) //update state and put latest sample into dive buffer when seal state changes
{
    if(BlubberLock.trylock_for(5s)) //try lock for 5 seconds
    {
        SEAL_STATE = newstate; //set member variable that main sampler thread can get
        PrintQueue.call(printf, "Seal state: %d\n\r", SEAL_STATE);

        sealsampleL4_t DiveSample; //new dive sample to go on characteristic dive buffer
        char timesample[32];
        time_t seconds = time(NULL); //get current time from the rtc
        SEAL_STATE = newstate; //set member variable that main sampler thread can get
        DiveSample.pressure = Pressure; //add pressure to sample
        DiveSample.temperature = Temperature; //add temp to sample
        strftime(timesample, 32, "%b:%d:%H:%M", localtime(&seconds)); //convert time to readable format
        DiveSample.time = timesample; //add time to sample
        DiveSample.state = newstate; //add state to sample
        Buffer->Put(DiveSample); //add sample to buffer!

        BlubberLock.unlock(); //release lock
    }
    else
    {
        PrintQueue.call(printf, "Blubberlock trylock failed in SetSealState \n\r"); //report problem
    }
}





