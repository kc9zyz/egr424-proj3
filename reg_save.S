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
  mov r1, sp

  //Store the registers not automatically saved by the interrupt
  //into the storage buffer
  stmea r0!, {r4-r12}

  //Store r1 in the last location because it contains the sp
  stmea r0!, {r1}

  //Return to the calling address
  bx lr