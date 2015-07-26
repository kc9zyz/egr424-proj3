/*
 * Implement locking
 */
    .syntax unified
    .text
    .align 2
    .thumb
    .thumb_func
    .type lock_acquire,function
    .global lock_acquire
    
lock_acquire:
    MOV R1, #0
    LDREX r2, [r0]
    CMP r2, r1
    ITT NE
    STREXNE r2, r1, [r0]

    CMPNE r2,#1
    BEQ 1f

    MOV R0, #1
    BX LR
 1:
    CLREX
    MOV R0, #0
    BX LR

