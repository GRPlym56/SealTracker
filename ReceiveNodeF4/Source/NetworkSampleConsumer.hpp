#ifndef _NET_SAMPLE_CONSUMER_H__
#define _NET_SAMPLE_CONSUMER_H__
#include "network.hpp"
class NetworkSampleConsumer: public SampleConsumer{
    public:
        NetworkSampleConsumer(Network* net);                        //initalise class with Network object referance
        void consumeSample(sample_t sample) override;               //override interface function and define consumeSample (put sample onto net buffer)
        void setConsumerContext(ConsumerContext* Context) override; //function not used in this class, implimentation left empty
    private:
        Network* net;                                               //store location of network object
};

#endif