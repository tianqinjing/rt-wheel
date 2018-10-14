/* sync-keil.h - sync library, including support for atomic, barrier keil version */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,16sep18,cfm  writen
*/

#ifndef __AARCH_M_SYNC_KEIL_H__
#define __AARCH_M_SYNC_KEIL_H__

/**
 * mb - read write memory barrier
 *
 * return: NA
 */

static inline void mb (void)
    {
    __dmb (0xf);
    }

/**
 * rmb - read memory barrier
 *
 * return: NA
 */

static inline void rmb (void)
    {
    __dmb (0xf);
    }

/**
 * wmb - write memory barrier
 *
 * return: NA
 */

static inline void wmb (void)
    {
    __dmb (0xf);
    }

/**
 * dsb - data synchronization barrier
 *
 * return: NA
 */

static inline void dsb (void)
    {
    __dsb (0xf);
    }

/**
 * isb - instruction synchronization barrier
 *
 * return: NA
 */

static inline void isb (void)
    {
    __isb (0xf);
    }

#endif  /* __AARCH_M_SYNC_KEIL_H__ */

