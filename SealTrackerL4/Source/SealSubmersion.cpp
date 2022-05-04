/*
    written by Guy Ringshaw 2022
*/

#include "SealSubmersion.hpp"
#include <chrono>


SealSubmersion::SealSubmersion(CircBuff* Buff, CommsWrapper* Communications, MS5837* PressSens): Buffer(Buff), NRF(Communications), Sensor(PressSens)
{

    GetAmbientDepth(); //grab initial pressure value

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
            PrintQueue.call(printf, "No data to send\n\r");
        }else{

            PrintQueue.call(printf, "Data available, attempting to send\n\r");
            NRF->On();
            unsigned int size = Buffer->GetSize();
            for (int i = 0; i<size; i++)
            {
                char message[32];
                sealsampleL4_t sample = Buffer->Get();
                sprintf(message, "%f,%f,%s", sample.pressure, sample.temperature, sample.time.c_str());
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
                //do nothing, delay is long enough
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
    
    PrintQueue.call(printf, "Delay: %d \n\r", delay);
    ThisThread::sleep_for(delay*1000);

}


void SealSubmersion::UpdateDepth() //measures current pressure value and updates depth
{
    depth[PREVIOUS] = depth[NOW]; //update prior value

    Sensor->Barometer_MS5837(); //update values
    float Pressure = Sensor->MS5837_Pressure();
    
    
    depth[NOW] = (Pressure - pressure_offset)/100.52; //remove 1013.25 ambient surface pressure, divide by 100.52 to get depth in metres
    PrintQueue.call(printf, "Depth: %f \n\r", depth[NOW]);

    delta_depth = depth[NOW] - depth[PREVIOUS];



}

void SealSubmersion::GetAmbientDepth() //convert ambient pressure value to depth to calibrate depth tracking
{

    Sensor->Barometer_MS5837();
    pressure_offset = Sensor->MS5837_Pressure();
    
}





