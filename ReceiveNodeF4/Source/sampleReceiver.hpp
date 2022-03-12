/*
ELEC-351 Group E
Author: Jacob Howell
December-January 2022
*/

#ifndef S_RECEIVE_H__
#define S_RECEIVE_H__
#include "Config.hpp"

class ConsumerContext{
    public:
        virtual int getDisplayParam() = 0;
        virtual Thresholds_t getThresholds() = 0;
        virtual void getThreshLock() = 0;
        virtual void releaseThreshLock() = 0;
        virtual void getParamLock() = 0;
        virtual void releaseParamLock() = 0;
        //virtual Watchdog getWatchdog() = 0;
};
class SampleConsumer{
    public:

        virtual void consumeSample(sample_t sample) = 0;  //
        virtual void setConsumerContext(ConsumerContext* Context) = 0;
};
#endif