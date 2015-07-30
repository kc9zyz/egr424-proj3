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

  //Restore the registers not automatically saved by the interrupt
  //from the storage buffer
  ldmia r0, {r1,r4-r12}

  //Move r1 into sp to restore the PSP
  msr psp, r1

  //Turn off GPIO port for context switching
  ldr	r3, =0x400253fc
  ldr	r2, [r3, #0]
  bic.w	r2, r2, #2
  str	r2, [r3, #0]
  //NOTE END CONTEXT SWITCH
  //fake a return from the handler to use
  //thread mode and process stack
  LDR LR, =0xFFFFFFFD
  bx lr
