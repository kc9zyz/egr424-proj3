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
    .type reg_restore,function
    .global reg_restore

reg_restore:

  //Load the stored PSP into r1
  ldmia r0!, {r1}

  //Restore the registers not automatically saved by the interrupt
  //from the storage buffer
  ldmia r0!, {r4-r12}

  //Move r1 into sp to restore the PSP
  mov sp, r1

  //Return to the calling address
  bx lr
