/* msg_queue.h - message queue header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,28jul18,cfm  writen
*/

#ifndef __MSG_QUEUE_H__
#define __MSG_QUEUE_H__

#include <stddef.h>

#include <wheel/list.h>

#include <kernel/sem.h>
#include <kernel/mutex.h>

#define MQ_OP_RD        0
#define MQ_OP_WT        1

typedef struct mq
    {
    sem_t   sem [2];
    mutex_t lock;
    size_t  msg_size;
    size_t  max_msgs;

    dlist_t msgs [2];
    } mq_t, * mq_id;

extern mq_id mq_create    (size_t msg_size, size_t max_msgs, unsigned int options);
extern int   mq_delete    (mq_id mq);
extern int   mq_send      (mq_id mq, void * buff, size_t size);
extern int   mq_timedsend (mq_id mq, void * buff, size_t size, unsigned int timeout);
extern int   mq_recv      (mq_id mq, void * buff, size_t size);
extern int   mq_timedrecv (mq_id mq, void * buff, size_t size, unsigned int timeout);

#endif  /* __MSG_QUEUE_H__ */

