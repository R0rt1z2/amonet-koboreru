.syntax unified

.thumb

.global start
.section .text.start
.type start, %function
.thumb_func
start:
    ldr  r12, [sp]
    push {r4-r7, lr}
    mov  r4, sp

    ldr  sp, =_stack_top
    sub  sp, sp, #8
    str  r12, [sp]

    ldr  r5, =__bss_start
    ldr  r6, =__bss_end
    movs r7, #0
0:  cmp  r5, r6
    bhs  1f
    str  r7, [r5], #4
    b    0b

1:  bl   main

    mov  sp, r4
    pop  {r4-r7, pc}
