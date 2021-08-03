#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"
#include "buzzer.h"
	
	.arch msp430g2553
	.p2align 1,0

    .text
    .global state_advance
    .extern reset_screen
    .extern buzzer_set_period

    .data
    .extern state
    .extern interrupts
    .extern secCount

temp:
    .word 3

jt:
	.word case_0
	.word case_1
	.word case_2
	.word case_3
	.word default

state_advance:
    cmp r12, &temp
    jc default

    add r12, r12
    mov jt(r12), r0

case_0:
    mov.b #0x1F, &interrupts
    jmp end
case_1:
    mov.b #0x3E, &interrupts
    jmp end
case_2:
    mov.b #0xB9, &interrupts
    jmp end
case_3:
    call #reset_screen
default:
    mov #0, &secCount
end:
    pop r0
