#include "scheduler.h"
unsigned countLock = 0;
unsigned lockedThread = 0;

//Locks
unsigned uartlock;

//Locks a resource by upcountLocking
unsigned lock(unsigned *threadlock)
{
        unsigned ret = 0;
        if(countLock) //resource is locked
        {
                //Check if the thread asking for the lock
                //is the thread that initially locked the resource
                if(lockedThread == get_currThread())
                        countLock++; //Increase the locked count
        }
        else //resource is unlocked
        {
                if(lock_acquire(threadlock)) //Try to get a lock
                {
                        lockedThread = get_currThread(); //Set the locking thread to this thread
                        countLock = 1; //Set the initial lock count to 1
                        ret = 1;
                }
        }
        return ret;
}
//Unlocks a resource by downcounting until 0
void unlock(unsigned *threadlock)
{
        //check to see if the thread is the original locker
        if(lockedThread == get_currThread())
                countLock--; //reduce the count
        if(!countLock) //if count is 0, unlock the resource
                lock_release(threadlock);
}

//Forces an unlock of the resource
void unlock_force(unsigned *threadlock)
{
        lock_release(threadlock);
        countLock = 0;
        lockedThread = 0;
}

void lock_release(unsigned *lock)
{
        asm volatile (
                "mov r1, #1\n"
                "str       r1, [r0]");
}
// These are functions you have to write. Right now they are do-nothing stubs.
void lock_init(unsigned *lock)
{
        asm volatile (
                "mov r1, #1\n"
                "str       r1, [r0]");
}
