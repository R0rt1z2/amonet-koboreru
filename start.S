.syntax unified

.thumb

.global start
.section .text.start
.type start, %function
.thumb_func
start:
    ldr r12, [sp]
    ldr sp, =_stack_top
    sub sp, sp, #8
    str r12, [sp]

    ldr r4, =__bss_start
    ldr r5, =__bss_end
    movs r6, #0
0:  cmp r4, r5
    bhs 1f
    str r6, [r4], #4
    b 0b

1:  bl main
2:  b 2b
