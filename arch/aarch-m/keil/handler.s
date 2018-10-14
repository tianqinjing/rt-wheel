; handler.s - exception/irq processing routine for aarch-m

;
; Copyright (c) 2018 Fangming Chai
;

;
;modification history
;--------------------
;01a,19aug18,cfm  writen
;

        EXPORT  irq_handler
        EXPORT  exc_handler

        IMPORT  hal_int_dispatch
        IMPORT  hal_exc_dispatch

        AREA    |.text|, CODE, READONLY

;
; irq_handler - irq handler when using irq dispatching
;
; return: NA
;

irq_handler
        PUSH    {r0, lr}
        MRS     r0, ipsr
        SUBS    r0, #16
        BL      hal_int_dispatch
        POP     {r0, pc}

;
; exc_handler - exception handler for all exceptions
;
; return: NA
;

exc_handler
        ; TODO: save esf!
        PUSH    {r0, lr}
        MRS     r0, ipsr
        BL      hal_exc_dispatch
        POP     {r0, pc}

        END

