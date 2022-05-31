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
        void Put(sealsampleL4_t newsample); //put new sample onto buffer
        sealsampleL4_t Get(void); //get oldest sample off buffer
        sealsampleL4_t Peek(void); //look at samples
        unsigned int GetSize(void);

        bool IsEmpty(void);

    private:

        Mutex Bufferlock; //Mutex lock for handling mutable data 

        
        
        const unsigned int max_size;              //size of buffer
        const unsigned int max_index;             //max index of buffer (size-1)
        volatile unsigned int head = 0;           //first element in buffer
        volatile unsigned int tail = 0;           //last element in buffer             
        volatile unsigned int currentsize = 0;    //buffer starts empty
        

        char * name; //buffer name
        
        sealsampleL4_t * Buffer; //pointer to buffer location when created by constructor


        void IncrementHead(void); //increase head index by one
        void IncrementTail(void); //increase tail index by one
        bool FullCheck();         //check if buffer is full
        bool EmptyCheck(void);    //check if buffer is empty
        
        unsigned int getsize(void); //return current size of buffer
        //void IncrementPointer(unsigned int& pointer);
       

};



#endif