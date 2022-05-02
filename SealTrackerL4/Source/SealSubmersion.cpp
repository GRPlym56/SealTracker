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
    //seeks to detect seal surfacing, as this is an opportunity to send data
    //however we dont want to be checking for it every second if the seal is 100m deep,
    //as it is unlikely that the seal will resurface within a second, especially since 
    //it is known that seals will often do dives that result in a V-shape on a depth graph

    //first algorithm attempt with perturb and observe style behaviour

    UpdateDepth(); //measure depth in metres and produce a delta

    if(depth[NOW] < 0.3f)
    {
        //seal is within margin of error, attempt to send data
        if(Buffer->IsEmpty())
        {
            //buffer empty, do nothing
        }else {

            NRF->On();

            while(!(Buffer->IsEmpty())){
                char message[32];
                sealsample_t sample = Buffer->Get();
                sprintf(message, "%s,%s,%s", sample.pressure.c_str(), sample.temperature.c_str(), sample.time.c_str());
                NRF->DataDump(message); //send sample
            }
            NRF->Off();
        }

        delay = 2; //set to minimum

    }else
    {

        //typecast delta depth to remove centimetre deviation, not important for 10 or 100m scale dive
        if((int)delta_depth == 0)
        {
            //seal is cruising, do not modify delay
        }else if((int)delta_depth > 0)
        {
            //seal has gone deeper, increase delay
            if(delay >= 60)
            {
                //do nothing, delay is short enough
            }else
            {
                delay += 2;
            }
        }else
        {
            //seal is rising, reduce delay
            if(delay <= 2)
            {
                //do nothing, delay is short enough
            }else
            {
                delay -= 4;
            }
        }
    }
    
    
    ThisThread::sleep_for(seconds_to_duration(delay));

}


void SealSubmersion::UpdateDepth() //measures current pressure value and updates depth
{
    depth[PREVIOUS] = depth[NOW]; //update prior value

    Sensor->Barometer_MS5837(); //update values
    float Pressure = Sensor->MS5837_Pressure();

    //TODO: replace 1013.25 with a value recorded on startup
    depth[NOW] = (Pressure - 1013.25)/100.52; //remove 1013.25 ambient surface pressure, divide by 100.52 to get depth in metres
    

    delta_depth = depth[NOW] - depth[PREVIOUS];



}

template <typename T>
auto seconds_to_duration(T seconds) 
{
    return std::chrono::duration<T, std::ratio<1>>(seconds);
}



