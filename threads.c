#include <stdio.h>
#include "scheduler.h"
#include "inc/lm3s6965.h"
#include "inc/hw_types.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "rit128x96x4.h"

volatile unsigned count;

// Thread 0 is currently an idle thread for the system to fall into
// when there are no other active threads
void thread0(void)
{
        iprintf("IDLE\r\n");
        while(1);
}
//Thread 1 is a UART thread demonstrating peripheral locking
//as well as calls to Yield();
void thread1(void)
{
        while (1) {
                //Try to get a resource lock to print out the message
                //across multiple yields.
                if (lock(&uartlock)) {
                        // Simulate code that is occasionally interrupted
                        iprintf("THIS IS T");
                        yield(); //Yield to scheduler
                        iprintf("HREAD NU");
                        yield(); //Yield to scheduler
                        iprintf("MBER 1\r\n");
                        unlock(&uartlock);
                }

                yield();
        }
}
//Thread 1 is a UART thread demonstrating peripheral locking
//as well as calls to Yield();
void thread2(void)
{
        while (1) {
                //Try to get a resource lock to print out the message
                //across multiple yields.
                if (lock(&uartlock)) {
                        // Simulate code that is occasionally interrupted
                        iprintf("this is t");
                        yield(); //Yield to scheduler
                        iprintf("hread number 2\r\n");

                        unlock(&uartlock);
                }
//If enabled, try to test the pirvilege level. If the thread is in unprivileged
//mode, the system will hard fault.
#ifdef PRIV_TEST
                NVIC_ST_CTRL_R = 0x00000000;
#endif
                yield();
        }
}

// This thread contains LED flashing commands and scheduler pre-emption
void thread3(void)
{
        //Set ETH LEDs out of sequence with eachother
        GPIO_PORTF_DATA_R ^= 0x04;
        while(1)
        {
                //Toggle the ethernet LED pins
                GPIO_PORTF_DATA_R ^= 0x0C;
                //Loop until timer0 ticks, not yielding voluntarily
                while(!TimerIntStatus(TIMER0_BASE,false)&TIMER_TIMA_TIMEOUT);
                //Clear the interrupt flag when the timer has ticked
                TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
        }
}

//Declare image variables
const unsigned char clear[] =
{
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
};
//Declare image variables
const unsigned char image[] =
{
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
// This thread will contain OLED commands
void thread4(void)
{
        //Declare variables for looping
        volatile int j, k;
        //Loop forever
        while(1)
        {
                //Draw the image of the block
                RIT128x96x4ImageDraw(image, 0, 48, 16, 3);
                //Wait 50 time slices
                for(j = 0; j < 50; j++)
                {
                        //Yield to scheduler
                        yield();
                }
                for(k = 8; k <= 112; k+=8)
                {
                        //Draw the image of the block
                        RIT128x96x4ImageDraw(image, k, 48, 16, 3);
                        //Clear the image of the previous block
                        RIT128x96x4ImageDraw(clear, (k-8), 48, 8, 3);
                        //Wait 50 time slices
                        for(j = 0; j < 50; j++)
                        {
                                //Yield to scheduler
                                yield();
                        }
                }
                //Draw the image of the block
                RIT128x96x4ImageDraw(image, 112, 48, 16, 3);
                //Wait 50 time slices
                for(j = 0; j < 50; j++)
                {
                        //Yield to scheduler
                        yield();
                }

                for(k = 104; k >= 0; k-=8)
                {
                        //Draw the image of the block
                        RIT128x96x4ImageDraw(image, k, 48, 16, 3);
                        //Clear the image of the previous block
                        RIT128x96x4ImageDraw(clear, (k+16), 48, 8, 3);
                        //Wait 50 time slices
                        for(j = 0; j < 50; j++)
                        {
                                //Yield to scheduler
                                yield();
                        }
                }
        }
}
