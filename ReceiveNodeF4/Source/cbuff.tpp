/*
ELEC-351 Group E
Author: Jacob Howell
December-January 2022
*/

//#include "cbuff.hpp"
#include <algorithm>
#include <string>


//constructor
template<class T>
Circular_Buff<T>::Circular_Buff (size_t size, char * buffName) : max_size(size), named(true), bufferName(buffName),full(0),empty(size){    //initalise member attribute to size
    buffer = new T [size];                               //allocate memory for buffer
    printf("Name: %s\tCapacity(samples):%d\r\n", bufferName, size);
}
template<class T>
Circular_Buff<T>::Circular_Buff (size_t size) : max_size(size) ,full(0),empty(size){    //initalise member attribute to size
    buffer = new T [size];                               //allocate memory for buffer
}
template<class T>
Circular_Buff<T>::Circular_Buff():max_size(128), full(0), empty(128){                   //overloaded constructor gives default size of 128 samples
    buffer = new T [128];
    this->full++;
}
//destructor
template<class T>
Circular_Buff<T>::~Circular_Buff(){                                //destructor deletes new dynamicly allocated memory
    delete buffer;                                              //delete array from heap to avoid memory leaks
}
template<class T>
int Circular_Buff<T>::put(T sample){                       //add new elements to buffer
    mutex.lock();                                               //lock muted in critical section
    if(isFull()){                                               //if the head is in the same position as the tail, the buffer is full
        //TODO Call error handler "Buffer overrun"
        if(hasName()){
            printf("%s Full\tThread ID: %d\t Thread Name: %s\r\n", bufferName, (int)ThisThread::get_id(), ThisThread::get_name());
        }else{printf("Unnamed Buffer is Full\tThread ID: %d\t Thread Name: %s\r\n", (int)ThisThread::get_id(), ThisThread::get_name());}
        mutex.unlock();  
        return -1;  //error buffer full
    }
    buffer[head] = sample;                                      //write sample to buffer array
    head = (head + 1) % max_size;                               //increment head by 1, and wrap around if greater than buffer size
    curr_size++;
    mutex.unlock();                                             //out of critical section, unlock mutex
    return 0;
}
template<class T>
T Circular_Buff<T>::peek(unsigned int index){               //allow external function to look inside buffer without changing it
    mutex.lock();
    sample_t sample;
    int i = (head - index);
    if (i < 0){
        i+=max_size;
    }
    sample = buffer[i];                          //return value of sample at specified index
    //PrintQueue.call(printf, "\ti:\t%d\th:\t%d\r\n", i,head);
    mutex.unlock();
    return sample;
}
template<class T>
T Circular_Buff<T>::get(){                                  //get last sample and remove it from buffer (move the tail)
    mutex.lock();                                               //lock mutex in critical section
    while(isEmpty()){                                              //check if the buffer is empty before getting sample from buffer
        //TODO Call error handeler "Buffer underrun"
        ThisThread::sleep_for(10s);
        //return sample_t();                                      //return empty sample
    }
    T sample = buffer[tail];                             //if buffer is not empty, return sample from buffer's tail
    curr_size--;                                                //subtract 1 from current size
    tail = (tail + 1) % max_size;                               //increment tail (includes wrapping so pointer stays within arrays limits)
    mutex.unlock();                                             //out of critical section, unlock mutex
    return sample;                                              //return sample
}
template<class T>
void Circular_Buff<T>::reset(){                                    //move head and tail to same position and reset current size. (emptying buffer)
    mutex.lock();                                               //lock mutex so no other threads can access whilst variables are being modified
    head = tail;                                                //
    curr_size = 0;                                              //  
    mutex.unlock(); //unlock mutex                              //unlock mutex
}
template<class T>
bool Circular_Buff<T>::isEmpty() const{                            //check if buffer is empty (for use external to the class)
    return (curr_size == 0);                                    //check size 
}
template<class T>
bool Circular_Buff<T>::isFull() const{                             //check if buffer is full (for use external to the class)
    return (curr_size >= max_size);     //check if full         //check current size to max size
}
template<class T>
size_t Circular_Buff<T>::getCapacity() const{                      //return capacity of buffer (for external use to the class) 
    return max_size;                                            //return size of buffer 
}
template<class T>
size_t Circular_Buff<T>::getSize() const{                          //return the current size of the buffer (for external use to the class)
    return curr_size;                               
}
template<class T>
std::string Circular_Buff<T>::getName(){
    return bufferName;
}
template<class T>
bool Circular_Buff<T>::hasName(){
    return named;
}
