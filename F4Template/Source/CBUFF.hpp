#ifndef __CBUFF__
#define __CBUFF__

#include "mbed.h"
#include "Config.hpp"
#include <cstddef>


extern EventQueue PrintQueue;

class CircBuff
{

    public:

        CircBuff(unsigned int buffsize, char * buffername);
   
        //public functions protected by Bufferlock
        void Put(sealsample_t newsample);
        sealsample_t Get(void);
        sealsample_t Peek(void);
        
        

    


    private:

        Mutex Bufferlock; //Mutex lock for handling mutable data 

        
        bool Full;
        const unsigned int max_size;          //size of buffer
        volatile unsigned int head = 0;           //first element in buffer
        volatile unsigned int tail = 0;           //last element in buffer              //needs to be mutex locked 
        volatile unsigned int currentsize = 0;
        

        char * name;
        
        sealsample_t * Buffer; //pointer to buffer location when created by constructor


        void IncrementHead(void);
        void IncrementTail(void);
        bool FullCheck();
        bool EmptyCheck(void);
        unsigned int getsize(void);
        //void IncrementPointer(unsigned int& pointer);
       

};



#endif