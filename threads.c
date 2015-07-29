#include <stdio.h>
#include "scheduler.h"
#include "inc/lm3s6965.h"
#include "inc/hw_types.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

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
  // while(1)
  // {
  //   yield();
  // }
}

// This thread will contain LED flashing commands
void thread3(void)
{
  volatile unsigned long ulLoop;
  //
  // Enable the GPIO port that is used for the on-board LED.
  //
  SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;

  //Enable timer0
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

  //
  // Do a dummy read to insert a few cycles after enabling the peripheral.
  //
  ulLoop = SYSCTL_RCGC2_R;

  //
  // Enable the GPIO pin for the LED (PF0).  Set the direction as output, and
  // enable the GPIO pin for digital function.
  //
  GPIO_PORTF_DIR_R = 0x01;
  GPIO_PORTF_DEN_R = 0x01;

  //Configure timer to run in timeout mode, at twice the sysclock frequency
  TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
  TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()/2);
  TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  TimerEnable(TIMER0_BASE, TIMER_A);



  while(1)
  {
    //Toggle the LED pin
    GPIO_PORTF_DATA_R ^= 0x01;
    //Loop until timer0 ticks
    while(!TimerIntStatus(TIMER0_BASE,false)&TIMER_TIMA_TIMEOUT)
      yield(); //Yield control to scheduler when timer has not ticked
    //Clear the interrupt flag when the timer has ticked
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  }
}

// This thread will contain OLED commands
void thread4(void)
{
  // while(1)
  // {
  //    yield();
  // }
}
