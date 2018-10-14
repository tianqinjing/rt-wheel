/* asm.h - assembly related definations */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,12aug18,cfm  writen
*/

#ifndef __ASM_H_
#define __ASM_H_

#ifndef __GNUC__
#error "only gcc/llvm is supported by this target!"
#endif

#define _PROC(proc, ...)        \
        __VA_ARGS__;            \
        .type proc, function;   \
        .syntax unified;        \
        .thumb;                 \
        .balign 4;              \
proc:

#define EXPORT(label)           .globl  label
#define IMPORT(label)           .extern label

/* define routine invoked from C */

#define PROC(proc)              _PROC(proc, .thumb)
#define ENDP(proc)              .size   proc, . - proc

#ifndef IMM
#define IMM #
#endif

#endif  /* __ASM_H_ */

