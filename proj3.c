#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "rit128x96x4.h"
#include "inc/lm3s6965.h"
#include "scheduler.h"

#define STACK_SIZE 4096   // Amount of stack space for each thread

typedef struct {
  int active;       // non-zero means thread is allowed to run
  char *stack;      // pointer to TOP of stack (highest memory location)
  int state[40];    // saved state for our custom save and restore functions
} threadStruct_t;
// thread_t is a pointer to function with no parameters and
// no return value...i.e., a user-space thread.
typedef void (*thread_t)(void);

// These are the external user-space threads. In this program, we create
// the threads statically by placing their function addresses in
// threadTable[]. A more realistic kernel will allow dynamic creation
// and termination of threads.
extern void thread0(void);
extern void thread1(void);
extern void thread2(void);
extern void thread3(void);
extern void thread4(void);

static thread_t threadTable[] = {
  thread0,
  thread1,
  thread2,
  thread3,
  thread4
};

#define NUM_THREADS (sizeof(threadTable)/sizeof(threadTable[0]))

// These static global variables are used in scheduler(), in
// the yield() function, and in threadStarter()
static threadStruct_t threads[NUM_THREADS]; // the thread table
int currThread;    // The currently active thread
void printFault();

// This is the handler for the systic timer that handles the scheduling
// of the threads.

void scheduler_Handler(void)
{
  //NOTE START CONTEXT SWITCH

	//disable interrupts
   IntMasterDisable();

	//save the state of the current thread
	//on the array of 10 elements
  if(currThread!=-1)
    reg_save(threads[currThread].state);


    if(++currThread == NUM_THREADS) {
      currThread = 1;
    }

    if (threads[currThread].active) {
      //Reset systick so that the next interrupt will delay as usual
       NVIC_ST_CURRENT_R = 0;

      //enable interrupts
       IntMasterEnable();

      //restore the state of the next thread
    	//from the array of 10 elements
      reg_restore(threads[currThread].state);

      //NOTE END CONTEXT SWITCH

    } else {
      yield();
    }

  // // No active threads left except our idle thread so jump to that.
  // currThread = 0;
  // reg_restore(threads[currThread].state);
}


// This function is called from within user thread context. It executes
// a jump back to the scheduler. When the scheduler returns here, it acts
// like a standard function return back to the caller of yield().
void yield(void)
{
    asm volatile ("svc #1");
}

// This is the starting point for all threads. It runs in user thread
// context using the thread-specific stack. The address of this function
// is saved by createThread() in the LR field of the jump buffer so that
// the first time the scheduler() does a longjmp() to the thread, we
// start here.
void threadStarter(void)
{

  // Call the entry point for this thread. The next line returns
  // only when the thread exits.
  (*(threadTable[currThread]))();

  // iprintf("%d ended\r\n",currThread);
  // Do thread-specific cleanup tasks. Currently, this just means marking
  // the thread as inactive. Do NOT free the stack here because we're
  // still using it! Remember, this function runs in user thread context.
  threads[currThread].active = 0;
  // This yield returns to the scheduler and never returns back since
  // the scheduler identifies the thread as inactive.
  yield();
}

// This function is implemented in assembly language. It sets up the
// initial jump-buffer (as would setjmp()) but with our own values
// for the stack (passed to createThread()) and LR (always set to
// threadStarter() for each thread).
extern void createThread(int *buf, char *stack);

void main(void)
{
  unsigned i;

  // Set the clocking to run directly from the crystal.
  SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                 SYSCTL_XTAL_8MHZ);

  // Initialize the OLED display.
  RIT128x96x4Init(1000000);

  // Enable the peripherals used by this example.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

  // Set GPIO A0 and A1 as UART pins.
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  // Configure the UART for 115,200, 8-N-1 operation.
  UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                      (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                       UART_CONFIG_PAR_NONE));
   // Start running coroutines
   // Setup SysTic clock to timeout every 0.5s with interrupt enabled
   IntMasterEnable();
   // Initialize the global thread lock
   lock_init(&uartlock);

  // Create all the threads and allocate a stack for each one
  for (i=0; i < NUM_THREADS; i++) {
    // Mark thread as runnable
    threads[i].active = 1;

    // Allocate stack
    threads[i].stack = (char *)malloc(STACK_SIZE) + STACK_SIZE;
    if (threads[i].stack == 0) {
      iprintf("Out of memory\r\n");
      exit(1);
    }

    // After createThread() executes, we can execute a jump
    // to threads[i].state and the thread will begin execution
    // at threadStarter() with its own stack.
    createThread(threads[i].state, threads[i].stack);
  }

  // Initialize curr_thread to idle thread 0
  currThread = 0;

  NVIC_ST_CTRL_R = 0;
  NVIC_ST_RELOAD_R = 8000;
  NVIC_ST_CURRENT_R = 0;
  NVIC_ST_CTRL_R = 0x00000007;



  yield();

  // If scheduler() returns, all coroutines are inactive and we return
  // from main() hence exit() should be called implicitly (according to
  // ANSI C). However, TI's startup_gcc.c code (ResetISR) does not
  // call exit() so we do it manually.
  exit(0);
}


void printFault()
{
  int R[16],j,psp,xpsr;
  char nib;
  char output[12];
  asm volatile("mov %0,r13":"=r" (R[13]));
  asm volatile("mov %0,r14":"=r" (R[14]));
  asm volatile("mov %0,r15":"=r" (R[15]));
  asm volatile("mrs %0, psp":"=r" (psp));
  asm volatile("mrs %0, xpsr":"=r" (xpsr));

 RIT128x96x4StringDraw("Fault",32,  0, 15);

//Print SP
for(j=0;j<8;j++)
{
  nib = ((R[13]>>(j*4)) &0xf);
  if(nib<10)
    output[10-j] = nib + '0';
  else
    output[10-j] =  (nib-10) + 'A';
}
output[0] = 's';
output[1] = 'p';
output[2] = ':';
output[11] = 0;
RIT128x96x4StringDraw(output,0, 15, 15);

//Print PSP
for(j=0;j<8;j++)
{
  nib = ((psp>>(j*4)) &0xf);
  if(nib<10)
    output[10-j] = nib + '0';
  else
    output[10-j] =  (nib-10) + 'A';
}
output[0] = 'P';
output[1] = 'S';
output[2] = ':';
output[11] = 0;
RIT128x96x4StringDraw(output,0, 30, 15);

//Print xpsr
for(j=0;j<8;j++)
{
  nib = ((xpsr>>(j*4)) &0xf);
  if(nib<10)
    output[10-j] = nib + '0';
  else
    output[10-j] =  (nib-10) + 'A';
}
output[0] = 'x';
output[1] = 'p';
output[2] = ':';
output[11] = 0;
RIT128x96x4StringDraw(output,0, 45, 15);

//Print LR
for(j=0;j<8;j++)
{
  nib = ((R[14]>>(j*4)) &0xf);
  if(nib<10)
    output[10-j] = nib + '0';
  else
    output[10-j] =  (nib-10) + 'A';
}
output[0] = 'l';
output[1] = 'r';
output[2] = ':';
output[11] = 0;
RIT128x96x4StringDraw(output,0, 60, 15);
}
/*
 * Compile with:
 * ${CC} -o lockdemo.elf -I${STELLARISWARE} -L${STELLARISWARE}/driverlib/gcc
 *     -Tlinkscript.x -Wl,-Map,lockdemo.map -Wl,--entry,ResetISR
 *     lockdemo.c create.S threads.c startup_gcc.c syscalls.c rit128x96x4.c
 *     -ldriver
 */
// vim: expandtab ts=2 sw=2 cindent
