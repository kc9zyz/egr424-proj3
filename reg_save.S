/*
 * Save the register state
 *
 * Saves r4->r12, PSP into the state buffer
 */
    .syntax unified
    .text
    .align 2
    .thumb
    .thumb_func
    .type reg_save,function
    .global reg_save

reg_save:
//NOTE START CONTEXT SWITCH
//Turn on the GPIO port for context switch timing
  ldr	r3, =0x400253fc //Address of GPIO_PORTF_DATA_R
  ldr	r2, [r3, #0]
  orr.w	r2, r2, #2 //Bitwise or with PF1
  str	r2, [r3, #0]

  //Move the PSP into r1 for storing to the buffer
  mrs r1, psp

  //Store the registers not automatically saved by the interrupt
  //into the storage buffer
  stmia r0, {r1,r4-r12}

  //Return to the calling address
  bx lr
