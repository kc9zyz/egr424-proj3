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
#include "driverlib/timer.h"

#define STACK_SIZE 4096   // Amount of stack space for each thread

typedef struct {
        int active; // non-zero means thread is allowed to run
        char *stack; // pointer to TOP of stack (highest memory location)
        int state[40]; // saved state for our custom save and restore functions
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
volatile int numActive;
volatile int firstRun;
void printFault();

//Set the privilege value of thread mode to unprivileged
//Only needs to be called at the beginning, because this
//sets the state of thread mode execution for the duration
//of execution
void unpriv(void)
{
        //Run code to change the thread mode pirvilege value
        asm volatile (
                "MRS R3, CONTROL\n"
                "ORR R3, R3, #1\n"
                "MSR CONTROL, R3\n"
                );

}
// This is the handler for the systic timer/SVC interrupt that
// handles the scheduling of the threads.
void scheduler_Handler(void)
{

        //disable interrupts
        IntMasterDisable();

        //save the state of the current thread
        //on the array of 10 elements
        if(!firstRun)
                reg_save(threads[currThread].state);
        else
                firstRun = 0;
        //check if there are active threads
        if(numActive)
        {
                if(++currThread == NUM_THREADS) {
                        currThread = 1;
                }

                if (threads[currThread].active) {
                        //Reset systick so that the next
                        //interrupt will delay as usual
                        NVIC_ST_CURRENT_R = 0;

                        //enable interrupts
                        IntMasterEnable();

                        //restore the state of the next thread
                        //from the array of 10 elements
                        reg_restore(threads[currThread].state);

                }
                else
                {
                        IntMasterEnable();
                        scheduler_Handler();
                }
        }
        else
        {
                // No active threads left so jump to idle.
                currThread = 0;
                reg_restore(threads[currThread].state);
        }

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
// is saved by createThread() in the LR field of the buffer so that
// the first time the scheduler() does a reg_restore to the thread, we
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

        //Decrease the number of active threads
        numActive--;
        // This yield returns to the scheduler and never returns back since
        // the scheduler identifies the thread as inactive.
        // while(1);
        yield();
}

//Access the currthread variable
int get_currThread(void)
{
        return currThread;
}

// This function is implemented in assembly language. It sets up the
// initial buffer (as would setjmp()) but with our own values
// for the stack (passed to createThread()) and LR (always set to
// threadStarter() for each thread).
extern void createThread(int *buf, char *stack);

//Main function of the program, sets up peripherals,
//creates threads, and starts the scheduler
void main(void)
{
        unsigned i;

        // Set the clocking to run directly from the crystal.
        SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_8MHZ);

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
        // Enable the GPIO pin for the LED0/1 (PF2/3). and PF1  Set the
        // direction as output, and enable the GPIO pin for digital function.
        //
        GPIO_PORTF_DIR_R = 0x0F;
        GPIO_PORTF_DEN_R = 0x0F;

        //Configure timer to run in timeout mode at twice the sysclock
        // frequency
        TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
        TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()/2);
        TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
        TimerEnable(TIMER0_BASE, TIMER_A);

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
        numActive = 0;
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
                if(i)
                        numActive++;
        }

        // Initialize curr_thread to idle thread 0
        currThread = 0;

        NVIC_ST_CTRL_R = 0;
        NVIC_ST_RELOAD_R = 8000;
        NVIC_ST_CURRENT_R = 0;
        NVIC_ST_CTRL_R = 0x00000007;

        //signal that it is the first run through the scheduler
        firstRun = 1;

        //Decrease privilige level for thread mode
        unpriv();

        yield();

        // If scheduler() returns, all coroutines are inactive and we return
        // from main() hence exit() should be called implicitly (according to
        // ANSI C). However, TI's startup_gcc.c code (ResetISR) does not
        // call exit() so we do it manually.
        exit(0);
}

//Prints out a fault message to the OLED screen
void printFault()
{
        int R[16],j,psp,xpsr,pc;
        char nib;
        char output[12];
        asm volatile ("mrs r1, psp");
        asm volatile ("LDMIA r1, {r0-r7}");
        asm volatile ("mov %0,r13" : "=r" (R[13]));
        asm volatile ("mov %0,r6" : "=r" (pc));
        asm volatile ("mov %0,r15" : "=r" (R[15]));
        asm volatile ("mrs %0, xpsr" : "=r" (xpsr));
        asm volatile ("mrs %0, psp" : "=r" (psp));

        RIT128x96x4StringDraw("Fault Occured: info",0,  0, 15);

        //Print SP
        for(j=0; j<8; j++)
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
        for(j=0; j<8; j++)
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
        for(j=0; j<8; j++)
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
        for(j=0; j<8; j++)
        {
                nib = ((pc>>(j*4)) &0xf);
                if(nib<10)
                        output[10-j] = nib + '0';
                else
                        output[10-j] =  (nib-10) + 'A';
        }
        output[0] = 'p';
        output[1] = 'c';
        output[2] = ':';
        output[11] = 0;
        RIT128x96x4StringDraw(output,0, 60, 15);
}
