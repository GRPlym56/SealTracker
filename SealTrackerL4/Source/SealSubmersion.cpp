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

    if(depth[NOW] < 0.1) //check if data transmission is possible
    {
        NRF->On();
        while(!Buffer->IsEmpty()) //repeat until buffer is empty
        {
            char message[32];
            sealsampleL4_t sample = Buffer->Get(); //get data off buffer
            sprintf(message, "%4.1f|%2.1f|%s|%d", sample.pressure, sample.temperature, sample.time.c_str(), sample.state); //format message
            NRF->SendmsgNoPwrCntrl(message); //send message
        }
        NRF->Off();
    }

    if(delta_depth > 1) //if depth has increased by 1m
    {
        SetSealState(sealstate_t::DIVING);
        delay++; //depth increasing, chance of surfacing soon becoming more unlikely
        
    }else if(delta_depth < -1) //if depth has decreased by 1m
    {
        SetSealState(sealstate_t::ASCENDING);
        delay -= 2; //reduce delay, but at a greater rate than the delay increase to improve reactivity

    }else //depth maintained within threshold, cruie
    {
        SetSealState(sealstate_t::CRUISING); 
        delay = 5; //reset delay, cruises are often shorter than dive/ascension periods
    }
    PrintQueue.call(printf, "Delay: %d\n\r", delay);
    ThisThread::sleep_for(delay*1000); 
}


void SealSubmersion::UpdateDepth() //measures current pressure value and updates depth
{
    depth[PREVIOUS] = depth[NOW]; //update prior value

    Sensor->Barometer_MS5837(); //update values
    float Pressure = Sensor->MS5837_Pressure();
    
    
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





