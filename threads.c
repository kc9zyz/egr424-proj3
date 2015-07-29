#include <stdio.h>
#include "scheduler.h"

volatile unsigned count;

// Thread 0 is currently an idle thread for the system to fall into
// when there are no other active threads
void thread0(void)
{
    iprintf("IDLE\r\n");
    while(1);
}

void thread1(void)
{
  while (1) {
      if (lock(&uartlock)) {
        // Simulate code that is occasionally interrupted
        iprintf("THIS IS T");
            yield(); // context switch "interrupt"
        iprintf("HREAD NU");
            yield(); // context switch "interrupt"
        iprintf("MBER 1\r\n");

        unlock(&uartlock);
      }
      yield();
    }
}

void thread2(void)
{
  while (1) {
    if (lock(&uartlock)) {
      // Simulate code that is occasionally interrupted
      iprintf("this is t");
          yield(); // context switch "interrupt"
      iprintf("hread number 2\r\n");

      unlock(&uartlock);
    }
    yield();
  }
}

// This thread will contain LED flashing commands
void thread3(void)
{
   //while(1)
   //{
  //   yield();
  // }
}

// This thread will contain OLED commands
void thread4(void)
{
  volatile int j, k;
  volatile unsigned char clear[] = {0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00};

  volatile unsigned char image[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

   while(1)
   {
     RIT128x96x4ImageDraw(image, 0, 48, 16, 3);
     for(j = 0 ; j < 50 ; j++)
     {
       yield();
     }

    for(k = 8 ; k <= 112 ; k+=8)
    {

      RIT128x96x4ImageDraw(image, k, 48, 16, 3);
      RIT128x96x4ImageDraw(clear, (k-8), 48, 8, 3);
      for(j = 0 ; j < 50 ; j++)
      {
        yield();
      }
    }
    RIT128x96x4ImageDraw(image, 112, 48, 16, 3);
    for(j = 0 ; j < 50 ; j++)
    {
      yield();
    }

   for(k = 104 ; k >= 0 ; k-=8)
   {

     RIT128x96x4ImageDraw(image, k, 48, 16, 3);
     RIT128x96x4ImageDraw(clear, (k+16), 48, 8, 3);
     for(j = 0 ; j < 50 ; j++)
     {
      yield();
     }
   }
 }
}
