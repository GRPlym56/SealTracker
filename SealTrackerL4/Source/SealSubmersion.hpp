#ifndef __SEALSUBMERSION_HPP__
#define __SEALSUBMERSION_HPP__

#include "mbed.h"
#include "platform/mbed_thread.h"
#include "CommsWrapper.hpp"
#include "SDWrapper.hpp"
#include "MS5837.h"


extern EventQueue PrintQueue;
typedef enum{NOW = 0, PREVIOUS = 1};
//extern MS5837 PressSens;

class SealSubmersion
{
    public: 

    SealSubmersion(CircBuff* SDBuff, CommsWrapper* Communications, MS5837* PressSens);
    void SurfaceDetection();
    void GetDepth();

    private:

    CircBuff* SDBuffer;
    CommsWrapper* NRF;
    MS5837* Sensor;

    volatile float depth[2]; //track previous and current depth
    volatile float delta_depth; //track difference in depth

    chrono::seconds delay = 5s; //default delay 5s



};



#endif