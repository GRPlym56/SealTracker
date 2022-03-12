/*
ELEC-351 Group E
Author: Jacob Howell
December-January 2022
*/

#ifndef C_BUFF
#define C_BUFF
#include "mbed.h"
#include "uop_msb.h"
#include "Config.hpp"
#include <cstddef>
#include "Config.hpp"

using namespace uop_msb;
extern EventQueue PrintQueue;

template<class T>
class Circular_Buff{

    //public attributes and member functions can be seen and used by anywhere in the software
    public:
        
        //constructor
        Circular_Buff();                //default constructor (default size of 128 samples)
        Circular_Buff (size_t size);    //initalise member attribute to size
        Circular_Buff(size_t size, char * buffName);
        
        //destructor
        ~Circular_Buff();               //destructor (deletes dynamicly allocated memory)

        int     put(T sample);  //add a new sample to the buffer
        T peek(unsigned int index);     //look inside buffer without changing it
        T get();                  //get the oldest sample from the buffer 
        void    reset();                //clear the buffer
        bool    isEmpty() const;        //check if empty
        bool    isFull() const;         //check if full
        size_t  getCapacity() const;    //returns max capacity 
        size_t  getSize() const;        //returns current number of samples in buffer
        std::string getName();
        bool    hasName();

    //private attributes and member functions that only this class can see and use
    private:
        Mutex mutex;                    //create mutex object to make buffer thread safe
        Semaphore full;
        Semaphore empty;
        char * bufferName;
        bool named = false;
        const size_t max_size;          //specify size of buffer
        volatile int head = 0;                   //store the location of the first element in buffer
        volatile int tail = 0;                   //store location of last element in buffer              //needs to be locked and volatile
        volatile size_t curr_size = 0;
        //bool full = false;
        
        T* buffer;                  //store location of buffer when created by constructor
};

#include "cbuff.tpp"
#endif
