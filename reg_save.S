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
  //Move the PSP into r1 for storing to the buffer
  mrs r1, psp

  //Store the registers not automatically saved by the interrupt
  //into the storage buffer
  stmea r0, {r1,r4-r12}

  //Return to the calling address
  bx lr
