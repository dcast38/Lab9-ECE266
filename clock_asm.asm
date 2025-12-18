;
; clockupdate.asm: ECE 266 Lab 3 Starter code, fall 2024
;
; Assembly code for updating the 7-seg to show a running clock.
;
; Created by Zhao Zhang
;

; include C header file
        .cdecls "clock.h"
        .text

; Declaration fields (pointers) to seg7 and ClockUpdate
_seg7           .field  seg7
_ClockUpdate    .field  ClockUpdate
_display_distance_mm    .field  display_distance_mm

        .global ClockUpdate
        .asmfunc
ClockUpdate
        PUSH    {LR, r0}        	; push return address and event

        ; check seg7.colon_on and turn it on/off
        LDR     r0, _seg7           ; load the address of seg7 to r0
        LDRB    r1, [r0, #4]        ; r1 = seg7.colon_on
        CMP     r1, #0              ; compare r1 and 0
        BEQ     turn_on_colon       ; if the colon is turned off, jump to the code turning it on
        MOVS    r1, #0              ; now the colon must have been turned on, so turn it off
        STRB    r1, [r0, #4]        ; store 0 to seg7.colon_on
        B       update_seg7_and_return   ; jump to return

turn_on_colon
        MOV     r1, #1              ; r1 = 1
        STRB    r1, [r0, #4]        ; store 1 to seg7.colon_on

        ; seg7.digit[0]

update_digits
        ; r0 currently holds &seg7

        ; Load the integer distance (0 9999) from C
        LDR     r6, _display_distance_mm   ; r6 = &display_distance_mm
        LDR     r6, [r6]                   ; r6 = display_distance_mm

        ; Optional: clamp to <= 9999
        MOVW    r1, #9999
        CMP     r6, r1
        IT      HI
        MOVHI   r6, r1

        ; We'll repeatedly divide by 10
        MOVS    r2, #10                    ; divisor = 10

        ; digit0 = r6 % 10
        UDIV    r3, r6, r2                 ; r3 = r6 / 10
        MLS     r4, r3, r2, r6             ; r4 = r6 - r3*10  (remainder)
        STRB    r4, [r0, #0]               ; seg7.digit[0]
        MOV     r6, r3                     ; r6 = quotient

        ; digit1 = r6 % 10
        UDIV    r3, r6, r2
        MLS     r4, r3, r2, r6
        STRB    r4, [r0, #1]               ; seg7.digit[1]
        MOV     r6, r3

        ; digit2 = r6 % 10
        UDIV    r3, r6, r2
        MLS     r4, r3, r2, r6
        STRB    r4, [r0, #2]               ; seg7.digit[2]
        MOV     r6, r3

        ; digit3 = r6 % 10
        UDIV    r3, r6, r2
        MLS     r4, r3, r2, r6
        STRB    r4, [r0, #3]               ; seg7.digit[3]

        B       update_seg7_and_return


update_seg7_and_return
        ; Physically update the 7-seg
        ;    Call Seg7Update(&seg)
        BL      Seg7Update          ; note that r0 = &seg7 at this time

        ; Schedule a callback after 0.5 seconds:
        ;   Call ScheduleCallback(event, event->time + 500);
        POP     {r0}                ; restore event and time
        LDR     r1, [r0, #8]        ; r1 = event->time
        ADD     r1, #500            ; time += 500
        BL      EventSchedule

        POP     {PC}                ; return
        .end

