#include <stdio.h>
#include "scheduler.h"

void thread1(void)
{
  while (1) {
    if (lockUart()) {
      // Simulate code that is occasionally interrupted
      iprintf("THIS IS T");
          yield(); // context switch "interrupt"
      iprintf("HREAD NU");
          yield(); // context switch "interrupt"
      iprintf("MBER 1\r\n");
      lockUart();
      unlockUart();
      unlockUart();
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
      if (lockUart()) {
        tryCount = 0;
        // Simulate code that is occasionally interrupted
        iprintf("this is t");
            yield(); // context switch "interrupt"
        iprintf("hread number 2\r\n");

        unlockUart();
      }
      yield();
    }
  //   tryCount = 0;
  //   unlockUart_force(); //Force unlock the uart if it fails 10 times
  // }
}
