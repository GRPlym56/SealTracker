#include "NetworkSampleConsumer.hpp"

NetworkSampleConsumer::NetworkSampleConsumer(Network* net):net(net){

}
void NetworkSampleConsumer::consumeSample(sample_t sample){     //add sample to network's buffer
    net->netBuff.put(sample);
}
void NetworkSampleConsumer::setConsumerContext(ConsumerContext *Context){   //not used
    //no context needed
}