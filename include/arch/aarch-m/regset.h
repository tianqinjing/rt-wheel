/* regset.c - register set defination for ARM arch-m serial */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,28jul18,cfm  writen
*/

#ifndef __AARCH_M_REGSET_H__
#define __AARCH_M_REGSET_H__

#include <stdint.h>

struct regset
    {
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t epsr;
    };

#define EPSR_T              (1 << 24)

#endif  /* __AARCH_M_REGSET_H__ */

