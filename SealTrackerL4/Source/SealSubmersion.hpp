/*
    written by Guy Ringshaw 2022
*/

#ifndef __SEALSUBMERSION_HPP__
#define __SEALSUBMERSION_HPP__

#include "mbed.h"
#include "platform/mbed_thread.h"
#include "CommsWrapper.hpp"
#include "SDWrapper.hpp"
#include "MS5837.h"


extern EventQueue PrintQueue;
typedef enum {NOW = 0, PREVIOUS = 1}delta;

template <typename T>
auto seconds_to_duration(T seconds); 
//extern MS5837 PressSens;

class SealSubmersion
{
    public: 

    SealSubmersion(CircBuff* SDBuff, CommsWrapper* Communications, MS5837* PressSens);
    void SurfaceDetection();
    void UpdateDepth();
    

    private:

    CircBuff* Buffer;
    CommsWrapper* NRF;
    MS5837* Sensor;

    volatile float depth[2]; //track previous and current depth
    volatile float delta_depth; //track difference in depth

    unsigned int delay = 5; //default delay 5s



};



#endif