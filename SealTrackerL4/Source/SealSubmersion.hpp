/*
    written by Guy Ringshaw 2022
*/

#ifndef __SEALSUBMERSION_HPP__
#define __SEALSUBMERSION_HPP__

#include "mbed.h"
#include "platform/mbed_thread.h"
#include "CommsWrapper.hpp"
#include "SDWrapper.hpp"
#include "Config.hpp"
#include "MS5837.h"



extern EventQueue PrintQueue;

typedef enum {NOW = 0, PREVIOUS = 1}delta;


class SealSubmersion
{
    public: 

    SealSubmersion(CircBuff* SDBuff, CommsWrapper* Communications, MS5837* PressSens);
    void SurfaceDetection();
    void UpdateDepth();
    void GetAmbientDepth();
    sealstate_t GetSealState();
    void SetSealState(sealstate_t newstate);


    private:

    Mutex BlubberLock;

    CircBuff* Buffer;
    CommsWrapper* NRF;
    MS5837* Sensor;

    volatile float Pressure;
    volatile float Temperature;
    volatile float depth[2]; //track previous and current depth
    volatile float delta_depth; //track difference in depth
    volatile float pressure_offset; //ambient pressure value in depth 

    unsigned int delay = 5000; //default delay 5000ms

    LowPowerTimer RestTimer; //low power ;)
    

    sealstate_t SEAL_STATE;
   



};



#endif