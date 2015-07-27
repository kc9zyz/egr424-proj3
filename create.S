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
  //Save registers to stack
  push {r4-r9, r10, r11}

  //*** Setup the stack to fit interrupt return format***

  //Setup xPSR
  mov r12, #0x0000
  movt r12, #0x0100

  //setup PC to start at threadStarter
  ldr  r11, .L0

  //Setup link register to initial value
  ldr  r10, =#0xFFFFFFFF

  //Setup r12
  mov r9, #0

  //Setup r3
  mov r8, #0

  //Setup r2
  mov r7, #0

  //Setup r1
  mov r6, #0

  //Setup r0
  mov r5, #0

/* This is the format of the process stack initially
  xPSR
  PC
  LR
  R12
  R3
  R2
  R1
  R0
*/
  //Store full descending the initial variables, into the process stack
  STMFD r1!, {r5-r12}

  //*** Setup the buffer ***

  //Store the registers not automatically saved by the interrupt
  //into the storage buffer
  stmea r0!, {r1,r4-r12}


  //restore registers from stack
  pop {r4-r9, r10, r11}

  //Return

  bx      lr

.L0:
    .word   threadStarter
