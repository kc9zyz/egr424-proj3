/*
 * Implement the thread creation task:
 *
 *   - initialize the jump buffer with appropriate values for
 *     R13 (stack) and R14 (first address to jump to)
 *   - all other registers are irrelevant upon thread creation
 *
 *   In the jump buffer, the R13 slot is set to the second parameter of this
 *   function (the top-of-stack address, passed in R1). The R14 slot is set to
 *   the address of the threadStarter() function.
 *
 *   The C prototype for this function call is:
 *       createThread(threads[i].state, threads[i].stack)
 *   thus:
 *       R0 <-- state (an array to store registers)
 *       R1 <-- stack (address of top-of-stack)
 */
    .syntax unified
    .text
    .align 2
    .thumb
    .thumb_func
    .type createThread,function
    .global createThread
createThread:

  //*** Setup the buffer ***

  //Store the registers not automatically saved by the interrupt
  //into the storage buffer
  stmea r0!, {r4-r12}



  //*** Setup the stack to fit interrupt return format***

  //load the stack pointer from r1
  ldr r2, [r1]

  //Setup xPSR
  mov.w r12 0x0
  movt r12 0x100
  ldr  r11, .L0 //load the address of threadStarter

  //Store r1 in the last location of the buff because it contains the sp
  stmea r0!, {r1}

  bx      lr

.L0:
    .word   threadStarter
