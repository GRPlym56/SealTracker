#include "CBUFF.hpp"
#include <algorithm>

CircBuff::CircBuff(unsigned int buffsize, char * buffername): max_size(buffsize), max_index(buffsize-1), name(buffername)
{

    Buffer = new sealsampleL4_t [buffsize]; //allocate some memory for the new buffer
    PrintQueue.call(printf, "New buffer %s, Size: %d\n\r", name, buffsize); //declare buffer creation


}


void CircBuff::Put(sealsampleL4_t newsample)
{
    
    //put a sample onto the buffer
    if(Bufferlock.trylock_for(10s))
    {
        if(FullCheck()) //check fullness
        {
            //buffer is full, overwrite oldest sample and update tail to represent next oldest sample
            IncrementTail(); //increment tail accordingly
            Buffer[head] = newsample; //put new sample onto the buffer
            IncrementHead(); //increment head accordingly
            //no need to change current size as buffer is still full

        }else 
        {
            Buffer[head] = newsample; //put new sample onto the buffer
            IncrementHead(); //increment head accordingly
            currentsize++; //increase size count
            
        }
        
        PrintQueue.call(printf, "%s Buffer Put: Size: %d, Head: %d, Tail: %d\n\r", name, currentsize, head, tail); //report buffer status
        Bufferlock.unlock(); //release lock
    }else 
    {
        PrintQueue.call(printf, "%s Fault: 'Put' trylock failed\n\r", name); //report problem
    }
    


}

sealsampleL4_t CircBuff::Get() //return a sample from the buffer and increment tail
{
    
    sealsampleL4_t returnsample;
    if(Bufferlock.trylock_for(15s))
    {
        if(EmptyCheck()) //check if buffer is empty
        {
            //nothing to do, buffer is empty

        }else
        {
            //buffer isnt empty, return sample
            returnsample = Buffer[tail]; //copy sample
            IncrementTail(); //increment tail position now that section of memory is 'free'
            currentsize--; //decrement size counter accordingly
            
        }
        


        PrintQueue.call(printf, "%s Buffer Get: Size: %d, Head: %d, Tail: %d\n\r", name, currentsize, head, tail); //report buffer status
        Bufferlock.unlock(); //release lock
    }else 
    {
        PrintQueue.call(printf, "%s Fault: 'Get' trylock failed\n\r", name); //report problem
    }
    return returnsample; //return result
}

sealsampleL4_t CircBuff::Peek()
{
    //return sample but dont increment tail
    sealsampleL4_t returnsample;
    if(Bufferlock.trylock_for(5s))
    {
        if(EmptyCheck()) //check if buffer is empty
        {
            //do nothing, buffer is empty

        }else
        {
            //buffer isnt empty, return sample
            returnsample = Buffer[tail];
            
        }
        
        PrintQueue.call(printf, "%s Buffer Peek: Size: %d, Head: %d, Tail: %d\n\r", name, currentsize, head, tail);
        Bufferlock.unlock();
    }else 
    {
        PrintQueue.call(printf, "%s Fault: 'Peek' trylock failed\n\r", name);
    }
    return returnsample;
}

bool CircBuff::FullCheck(void) //private function, no mutex since I control when it happens
{
    if(currentsize == max_size)
    {
        PrintQueue.call(printf, "%s buffer full\n\r", name);
        return 1; //buffer full, return true
        
    }else 
    {
        return 0; //buffer not full, return false
    }
    
}

bool CircBuff::EmptyCheck(void) //private function, no mutex since i control when it happens
{
   
    if(currentsize == 0)
    {
        PrintQueue.call(printf, "%s buffer empty\n\r", name);
        return 1; //buffer empty
        
    }else 
    {
        return 0; //buffer not empty
    }

        
}

bool CircBuff::IsEmpty(void) //public version with mutex since it can happen anywhere
{
    if(Bufferlock.trylock_for(5s))
    {
        if(currentsize == 0)
        {
            Bufferlock.unlock();
            PrintQueue.call(printf, "%s buffer empty\n\r", name);
            return 1; //buffer empty
            
        }else 
        {
            Bufferlock.unlock();
            return 0; //buffer not empty
        }
        
    }else 
    {
        PrintQueue.call(printf, "%s Fault: 'IsEmpty' trylock failed\n\r", name);
        return 1; //effectively empty if fault occurs
    }
}

void CircBuff::IncrementHead(void) //private function, no mutex
{
    if(head == max_index) 
    {
        head = 0; //however we want the start of the buffer, 0 not 1
    }else  
    {
        head++;
    }
    
}

void CircBuff::IncrementTail(void) //private function, no mutex
{
    if(tail == max_index) 
    {
        tail = 0; //however we want the start of the buffer, 0 not 1
    }else  
    {
        tail++;
    }
    
}

unsigned int CircBuff::GetSize(void) //get buffer size
{
    
    if(Bufferlock.trylock_for(5s))
    {
        unsigned int Size = currentsize; //make a copy so lock can be released
        Bufferlock.unlock(); //release lock
        return Size; //return count
    }else 
    {
        PrintQueue.call(printf, "%s Fault: 'GetSize' trylock failed\n\r", name); //report problem
        return 0;
    }
}

/*
//increment in place, test later
void CircBuff::IncrementPointer(unsigned int& pointer)
{
    if((pointer + 1) % max_size) //returns 1 if pointer wraps around
    {
        pointer = 0; //however we want the start of the buffer, 0 not 1
    }else  //r
    {
        pointer++;
    }
    
}
*/

