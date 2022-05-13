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
   

    UpdateDepth(); //measure depth in metres and produce a delta

    if(depth[NOW] < 0.1)
    {
        if(!Buffer->IsEmpty()){ //check if buffer is empty before turning comms on

            NRF->On();
            do 
            {
                char message[32];
                sealsampleL4_t sample = Buffer->Get(); //get data off buffer
                sprintf(message, "%4.1f|%2.1f|%s|%d", sample.pressure, sample.temperature, sample.time.c_str(), sample.state); //format message
                NRF->SendmsgNoPwrCntrl(message); //send message  
            }while(!Buffer->IsEmpty()); //repeat until buffer is empty

            NRF->Off();
        }
        RestTimer.start();
        if(RestTimer.elapsed_time() >= 60s)
        {
            //seal is probably hauled out
            SetSealState(sealstate_t::RESTING);
            RestTimer.stop();
            RestTimer.reset();

            delay = 30000; //if seal is hauled out then nothing exciting will happen anytime soon, slow down
        }else 
        {
            SetSealState(sealstate_t::SURFACE);
            delay = 5000; //reset delay as seal may be about to dive or haul
        }
        
            
    }else if(delta_depth > 1 ) //seal diving?
    {
        RestTimer.stop();
        RestTimer.reset();
        SetSealState(sealstate_t::DIVING);
        
        delay = 5000; //moderate day to keep track of the dive
        
    }else if(delta_depth < 1) //seal ascending?
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
        delay = ((unsigned int)depth*3000)/10; //larger delay the deeper the seal is (example: 100m depth = (100*3)/10 = 30s delay)
    }

    ThisThread::sleep_for(delay);
}

void SealSubmersion::UpdateDepth() //measures current pressure value and updates depth
{
    //update prior value
    depth[PREVIOUS] = depth[NOW]; 
    

    Sensor->Barometer_MS5837(); //update values
    float Pressure = Sensor->MS5837_Pressure(); //get pressure value from sensor class
    
    
    depth[NOW] = (Pressure - pressure_offset)/100.52; //remove ambient surface pressure, divide by 100.52 to get depth in metres
    PrintQueue.call(printf, "Depth: %f \n\r", depth[NOW]);

    delta_depth = depth[NOW] - depth[PREVIOUS];
    


}

void SealSubmersion::GetAmbientDepth() //convert ambient pressure value to depth to calibrate depth tracking
{

    Sensor->Barometer_MS5837();
    pressure_offset = Sensor->MS5837_Pressure();
    PrintQueue.call(printf, "Ambient pressure: %4.2f \n\r", pressure_offset);
    
}

sealstate_t SealSubmersion::GetSealState() //returns current state of seal
{
   
    if(BlubberLock.trylock_for(5s)) //try mutex for 5s
    {
        sealstate_t state = SEAL_STATE;
        BlubberLock.unlock();
        return state; 
    }else 
    {
        PrintQueue.call(printf, "BlubberLock trylock failed in GetSealState \n\r");
        return sealstate_t::UNKNOWN;
    }

}

void SealSubmersion::SetSealState(sealstate_t newstate)
{
    if(BlubberLock.trylock_for(5s))
    {
        SEAL_STATE = newstate;
        BlubberLock.unlock();
    }
    else
    {
        PrintQueue.call(printf, "Blubberlock trylock failed in SetSealState \n\r");
    }
}





