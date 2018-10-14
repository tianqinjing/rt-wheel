/* irq.c - irq abstract layer header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,28jul18,cfm  writen
*/

#ifndef __IRQ_H__
#define __IRQ_H__

extern unsigned int  int_cnt;

extern unsigned long int_lock   (void);
extern void          int_unlock (unsigned long flags);

#endif  /* __IRQ_H__ */

