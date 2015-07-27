#include <stdio.h>
#include "scheduler.h"

volatile unsigned count;

// Thread 0 is currently an idle thread for the system to fall into
// when there are no other active threads
void thread0(void)
{
  while(1)
    iprintf("IDLE\r\n");
}

void thread1(void)
{

  count = 0;
  while(1)
  {
    if(lock(&uartlock))
    {
      iprintf("%d -> %d\r\n",currThread, count++);
      unlock(&uartlock);
    }
    // yield();
  }

}

void thread2(void)
{
  while(1)
  {
    yield();
  }
}

// This thread will contain LED flashing commands
void thread3(void)
{
  while(1)
  {
    yield();
  }
}

// This thread will contain OLED commands
void thread4(void)
{
  while(1)
  {
     yield();
  }
}
