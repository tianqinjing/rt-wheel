/* exc.h - arm-m arch exception abstract library header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,22sep18,cfm  writen
*/

#ifndef __AARCH_M_EXC_H__
#define __AARCH_M_EXC_H__

/* defines */

#define EXC_VEC_RESET               1
#define EXC_VEC_NMI                 2
#define EXC_VEC_HARDFAULT           3
#define EXC_VEC_MEMMANAGE           4
#define EXC_VEC_BUSFAULT            5
#define EXC_VEC_USABEFAULT          6
#define EXC_VEC_SVC                 11
#define EXC_VEC_DEBUGMONITOR        12
#define EXC_VEC_PENDSV              14
#define EXC_VEC_SYSTICK             15

/* typedefs */

typedef struct exc_info
    {
    // TODO: enrich me
    uint32_t     vector;
    uint32_t     dfar;             /* dfar or dummy */
    uint32_t     dfsr;             /* dfsr or dummy */
    uint32_t     ifar;             /* ifar or dummy */
    uint32_t     ifsr;             /* ifsr or dummy */
    struct
        {
        uintptr_t sp;
        uintptr_t r[13];
        uintptr_t lr;
        uintptr_t pc;
        uintptr_t cpsr;
        } context;
    } exc_info_t;

#endif  /* __AARCH_M_EXC_H__ */

