#include "scheduler.h"
unsigned count = 0;
unsigned lockedThread = 0;

//Locks a resource by upcounting
unsigned lockUart()
{
  unsigned ret = 0;
  if(count) //resource is locked
  {
    //Check if the thread asking for the lock
    //is the thread that initially locked the resource
    if(lockedThread == currThread)
      count++; //Increase the locked count
  }
  else //resource is unlocked
  {
    if(lock_acquire(&threadlock)) //Try to get a lock
    {
      lockedThread = currThread; //Set the locking thread to this thread
      count = 1; //Set the initial lock count to 1
      ret = 1;
    }
  }
  return ret;
}
//Unlocks a resource by downcounting until 0
void unlockUart()
{
  //check to see if the thread is the original locker
  if(lockedThread == currThread)
    count--; //reduce the count
  if(!count) //if count is 0, unlock the resource
    lock_release(&threadlock);
}

//Forces an unlock of the resource
void unlockUart_force()
{
  lock_release(&threadlock);
  count = 0;
  lockedThread = 0;
}
