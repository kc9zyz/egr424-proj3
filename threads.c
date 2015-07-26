#include <stdio.h>
#include "scheduler.h"

// Thread 0 is currently an idle thread for the system to fall into
// when there are no other active threads
void thread0(void)
{
  while(1);
}

void thread1(void)
{
  while (1) {
    if (lock(uartlock)) {
      // Simulate code that is occasionally interrupted
      iprintf("THIS IS T");
          yield(); // context switch "interrupt"
      iprintf("HREAD NU");
          yield(); // context switch "interrupt"
      iprintf("MBER 1\r\n");
      lock(uartlock);
      unlock(uartlock);
      unlock(uartlock);
    }
    yield();
  }
}

void thread2(void)
{
  unsigned tryCount = 0;
  while (1) {
    // while(tryCount++ <10)
    // {
      if (lock(uartlock)) {
        tryCount = 0;
        // Simulate code that is occasionally interrupted
        iprintf("this is t");
            yield(); // context switch "interrupt"
        iprintf("hread number 2\r\n");

        unlock(uartlock);
      }
      yield();
    }
  //   tryCount = 0;
  //   unlockUart_force(); //Force unlock the uart if it fails 10 times
  // }
}

// This thread will contain LED flashing commands
void thread3(void)
{
  yield();
}

// This thread will contain OLED commands
void thread4(void)
{
  yield();
}
