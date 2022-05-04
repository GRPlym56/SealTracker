#include "CBUFF.hpp"
#include <algorithm>

CircBuff::CircBuff(unsigned int buffsize, char * buffername): max_size(buffsize), max_index(buffsize-1), name(buffername)
{

    Buffer = new sealsampleL4_t [buffsize]; //allocate some memory for the new buffer
    PrintQueue.call(printf, "New buffer %s, Size: %d\n\r", name, buffsize);


}


void CircBuff::Put(sealsampleL4_t newsample)
{
    
    //put a sample onto the buffer
    if(Bufferlock.trylock_for(10s))
    {
        if(FullCheck())
        {
            //buffer is full, overwrite oldest sample and update tail to represent next oldest sample
            IncrementTail();
            Buffer[head] = newsample;
            IncrementHead();
            //no need to change current size as buffer is full

        }else 
        {
            Buffer[head] = newsample;
            IncrementHead();
            currentsize++;
            
        }
        
        PrintQueue.call(printf, "%s Buffer Put: Size: %d, Head: %d, Tail: %d\n\r", name, currentsize, head, tail);
        Bufferlock.unlock();
    }else 
    {
        PrintQueue.call(printf, "%s Fault: 'Put' trylock failed\n\r", name);
    }
    


}

sealsampleL4_t CircBuff::Get() //return a sample from the buffer and increment tail
{
    
    sealsampleL4_t returnsample;
    if(Bufferlock.trylock_for(15s))
    {
        if(EmptyCheck()) //check if buffer is empty
        {
            //do nothing, buffer is empty

        }else
        {
            //buffer isnt empty, return sample
            returnsample = Buffer[tail];
            IncrementTail();
            currentsize--; //decrement size counter
            
        }
        


        PrintQueue.call(printf, "%s Buffer Get: Size: %d, Head: %d, Tail: %d\n\r", name, currentsize, head, tail);
        Bufferlock.unlock();
    }else 
    {
        PrintQueue.call(printf, "%s Fault: 'Get' trylock failed\n\r", name);
    }
    return returnsample;
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

bool CircBuff::FullCheck(void) //private function, no mutex
{
    if(currentsize == max_size)
    {
        PrintQueue.call(printf, "%s buffer full\n\r", name);
        return 1; //buffer full
        
    }else 
    {
        return 0; //buffer not full
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
            PrintQueue.call(printf, "%s buffer empty\n\r", name);
            return 1; //buffer empty
            
        }else 
        {
            return 0; //buffer not empty
        }
        Bufferlock.unlock();
    }else 
    {
        PrintQueue.call(printf, "%s Fault: 'IsEmpty' trylock failed\n\r", name);
        return 1; //effectively empty if fault occurs
    }
}

void CircBuff::IncrementHead(void) //private function, no mutex
{
    if(head == max_index) //returns 1 if pointer wraps around
    {
        head = 0; //however we want the start of the buffer, 0 not 1
    }else  
    {
        head++;
    }
    
}

void CircBuff::IncrementTail(void) //private function, no mutex
{
    if(tail == max_index) //returns 1 if pointer wraps around
    {
        tail = 0; //however we want the start of the buffer, 0 not 1
    }else  
    {
        tail++;
    }
    
}

unsigned int CircBuff::GetSize(void) //
{
    if(Bufferlock.trylock_for(5s))
    {
        return currentsize;
        Bufferlock.unlock();
    }else 
    {
        PrintQueue.call(printf, "%s Fault: 'GetSzie' trylock failed\n\r", name);
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

