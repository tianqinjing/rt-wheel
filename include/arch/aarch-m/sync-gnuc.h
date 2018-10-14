/* sync-gnuc.h - sync library, including support for atomic, barrier gnuc version */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,16sep18,cfm  writen
*/

#ifndef __AARCH_M_SYNC_GNUC_H__
#define __AARCH_M_SYNC_GNUC_H__

/**
 * mb - read write memory barrier
 *
 * return: NA
 */

static inline void mb (void)
    {
    __asm__ __volatile__ ("dmb #0xf" : : : "memory");
    }

/**
 * rmb - read memory barrier
 *
 * return: NA
 */

static inline void rmb (void)
    {
    __asm__ __volatile__ ("dmb #0xf" : : : "memory");
    }

/**
 * wmb - write memory barrier
 *
 * return: NA
 */

static inline void wmb (void)
    {
    __asm__ __volatile__ ("dmb #0xf" : : : "memory");
    }

/**
 * dsb - data synchronization barrier
 *
 * return: NA
 */

static inline void dsb (void)
    {
    __asm__ __volatile__ ("dsb #0xf" : : : "memory");
    }

/**
 * isb - instruction synchronization barrier
 *
 * return: NA
 */

static inline void isb (void)
    {
    __asm__ __volatile__ ("isb #0xf" : : : "memory");
    }

#endif  /* __AARCH_M_SYNC_GNUC_H__ */

