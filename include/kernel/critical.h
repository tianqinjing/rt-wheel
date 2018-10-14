/* critical.h - critical job module */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,19aug18,cfm  writen
*/

#ifndef __CRITICAL_H__
#define __CRITICAL_H__

#include <stdint.h>

extern int do_critical_might_sleep (int (* job) (uintptr_t, uintptr_t),
                                    uintptr_t arg1, uintptr_t arg2);
extern int do_critical             (int (* job) (uintptr_t, uintptr_t),
                                    uintptr_t arg1, uintptr_t arg2);
extern int do_critical_non_irq     (int (* job) (uintptr_t, uintptr_t),
                                    uintptr_t arg1, uintptr_t arg2);

#endif  /* __CRITICAL_H__ */

