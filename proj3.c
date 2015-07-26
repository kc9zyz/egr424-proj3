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
#include "scheduler.h"

#define STACK_SIZE 4096   // Amount of stack space for each thread


typedef struct {
  int active;       // non-zero means thread is allowed to run
  char *stack;      // pointer to TOP of stack (highest memory location)
  int state[23];    // saved state for longjmp()
} threadStruct_t;

// thread_t is a pointer to function with no parameters and
// no return value...i.e., a user-space thread.
typedef void (*thread_t)(void);

// These are the external user-space threads. In this program, we create
// the threads statically by placing their function addresses in
// threadTable[]. A more realistic kernel will allow dynamic creation
// and termination of threads.
extern void thread1(void);
extern void thread2(void);

static thread_t threadTable[] = {
  thread1,
  thread2
};
#define NUM_THREADS (sizeof(threadTable)/sizeof(threadTable[0]))

// These static global variables are used in scheduler(), in
// the yield() function, and in threadStarter()
static jmp_buf scheduler_buf;   // saves the state of the scheduler
static threadStruct_t threads[NUM_THREADS]; // the thread table
unsigned currThread;    // The currently active thread

// This is the handler for the systic timer that handles the scheduling 
// of the threads.
void scheduler_Handler(void)
{
	//disable interrupts
  IntMasterDisable();

	//save the state of the current thread
	//on the array of 10 elements
  reg_save(threads[currThread].state);

	//identify the next active thread
  if (! threads[currThread].active) {
    free(threads[currThread].stack - STACK_SIZE);
  }

  unsigned i;

  currThread = -1;

  // We saved the state of the scheduler, now find the next
  // runnable thread in round-robin fashion. The 'i' variable
  // keeps track of how many runnable threads there are. If we
  // make a pass through threads[] and all threads are inactive,
  // then 'i' will become 0 and we can exit the entire program.
  i = NUM_THREADS;
  do {
    // Round-robin scheduler
    if (++currThread == NUM_THREADS) {
      currThread = 0;
    }

    if (threads[currThread].active) {
      //restore the state of the next thread
    	//from the array of 10 elements
      reg_restore(threads[currThread].state);

    	//fake a return from the handler to use
    	//thread mode and process stack

    	//enable interrupts
      IntMasterEnable();
    } else {
      i--;
    }
  } while (i > 0);

  // No active threads left. Leave the scheduler, hence the program.
  exit(0);
}

// This function is called from within user thread context. It executes
// a jump back to the scheduler. When the scheduler returns here, it acts
// like a standard function return back to the caller of yield().
void yield(void)
{
  if (setjmp(threads[currThread].state) == 0) {
    // yield() called from the thread, jump to scheduler context
    longjmp(scheduler_buf, 1);
  } else {
    // longjmp called from scheduler, return to thread context
    return;
  }
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
extern void createThread(jmp_buf buf, char *stack);

// This is the "main loop" of the program.
void scheduler(void)
{
  unsigned i;

  currThread = -1;

  do {
    // It's kinda inefficient to call setjmp() every time through this
    // loop, huh? I'm sure your code will be better.
    if (setjmp(scheduler_buf)==0) {

      // We saved the state of the scheduler, now find the next
      // runnable thread in round-robin fashion. The 'i' variable
      // keeps track of how many runnable threads there are. If we
      // make a pass through threads[] and all threads are inactive,
      // then 'i' will become 0 and we can exit the entire program.
      i = NUM_THREADS;
      do {
        // Round-robin scheduler
        if (++currThread == NUM_THREADS) {
          currThread = 0;
        }

        if (threads[currThread].active) {
          longjmp(threads[currThread].state, 1);
        } else {
          i--;
        }
      } while (i > 0);

      // No active threads left. Leave the scheduler, hence the program.
      return;

    } else {
      // yield() returns here. Did the thread that just yielded to us exit? If
      // so, clean up its entry in the thread table.
      if (! threads[currThread].active) {
        free(threads[currThread].stack - STACK_SIZE);
      }
    }
  } while (1);
}

void main(void)
{
  unsigned i;

  // Set the clocking to run directly from the crystal.
  SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                 SYSCTL_XTAL_8MHZ);

  // Initialize the OLED display and write status.
  RIT128x96x4Init(1000000);
  RIT128x96x4StringDraw("Scheduler Demo",       20,  0, 15);

  // Enable the peripherals used by this example.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

  // Set GPIO A0 and A1 as UART pins.
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  // Configure the UART for 115,200, 8-N-1 operation.
  UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                      (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                       UART_CONFIG_PAR_NONE));

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

    // After createThread() executes, we can execute a longjmp()
    // to threads[i].state and the thread will begin execution
    // at threadStarter() with its own stack.
    createThread(threads[i].state, threads[i].stack);
  }

  // Initialize the global thread lock
  lock_init(&uartlock);

  // Start running coroutines
  scheduler();

  // If scheduler() returns, all coroutines are inactive and we return
  // from main() hence exit() should be called implicitly (according to
  // ANSI C). However, TI's startup_gcc.c code (ResetISR) does not
  // call exit() so we do it manually.
  exit(0);
}

/*
 * Compile with:
 * ${CC} -o lockdemo.elf -I${STELLARISWARE} -L${STELLARISWARE}/driverlib/gcc
 *     -Tlinkscript.x -Wl,-Map,lockdemo.map -Wl,--entry,ResetISR
 *     lockdemo.c create.S threads.c startup_gcc.c syscalls.c rit128x96x4.c
 *     -ldriver
 */
// vim: expandtab ts=2 sw=2 cindent
