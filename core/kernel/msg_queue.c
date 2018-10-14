/* msg_queue.c - message queue implementation */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,28jul18,cfm  writen
*/

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>

#include <arch/config.h>            /* for ALLOC_ALIGN */

#include <wheel/common.h>

#include <kernel/msg_queue.h>

/**
 * mq_create - create a message queue
 * @msg_size: the message size of the queue
 * @max_msgs: the max number of messages in the queue
 * @options:  the options, not used for now
 *
 * return: the message queue id on success, NULL on error
 */

mq_id mq_create (size_t msg_size, size_t max_msgs, unsigned int options)
    {
    mq_id     mq;
    size_t    alloc_size;
    dlist_t * msg_head;

    (void) options;     // TODO: not used for now

    if ((msg_size == 0) || (max_msgs == 0))
        {
        return NULL;
        }

    msg_size = round_up (msg_size, ALLOC_ALIGN);

    alloc_size = sizeof (mq_t) + (msg_size + sizeof (dlist_t)) * max_msgs;

    mq = (mq_id) malloc (alloc_size);

    if (mq == NULL)
        {
        return NULL;
        }

    sem_init (&mq->sem [MQ_OP_RD], 0);
    sem_init (&mq->sem [MQ_OP_WT], max_msgs);

    mutex_init (&mq->lock);

    mq->msg_size = msg_size;
    mq->max_msgs = max_msgs;

    dlist_init (&mq->msgs [MQ_OP_RD]);
    dlist_init (&mq->msgs [MQ_OP_WT]);

    msg_head = (dlist_t *) (mq + 1);

    do
        {
        dlist_add (&mq->msgs [MQ_OP_WT], msg_head);
        max_msgs--;
        msg_head = (dlist_t *) (((char *) msg_head) + sizeof (dlist_t) + msg_size);
        } while (max_msgs);

    return mq;
    }

/**
 * mq_delete - delete a message queue
 * @mq: the message queue to be deleted
 *
 * return: 0 on success, negtive value on error
 */

int mq_delete (mq_id mq)
    {
    return -1;  // TODO:
    }

static inline int __mq_transfer (mq_id mq, void * buff, size_t size,
                                 unsigned int op, unsigned int timeout)
    {
    dlist_t * head;

    if (!mq || !buff || !size || op > MQ_OP_WT)
        {
        return -1;
        }

    size = min (size, mq->msg_size);

    if (sem_timedwait (&mq->sem [op], timeout))
        {
        return -1;
        }

    if (mutex_lock (&mq->lock))
        {
        return -1;
        }

    head = mq->msgs [op].next;

    dlist_del (head);

    if (op == MQ_OP_RD)
        {
        memcpy (buff, head + 1, size);
        }
    else
        {
        memcpy (head + 1, buff, size);
        }

    dlist_add (&mq->msgs [1 - op], head);

    (void) mutex_unlock (&mq->lock);

    (void) sem_post (&mq->sem [1 - op]);

    return 0;
    }

/**
 * mq_send - send a message from to message queue
 * @mq:   the message queue
 * @buff: the receive buffer
 * @size: the buffer size
 *
 * return: 0 on success, negtive value on error
 */

int mq_send (mq_id mq, void * buff, size_t size)
    {
    return __mq_transfer (mq, buff, size, MQ_OP_WT, UINT_MAX);
    }

/**
 * mq_timedsend - send a message from to message queue with timeout
 * @mq:   the message queue
 * @buff: the buffer holding the message to send
 * @size: the buffer size
 * @timeout: the max number of waiting ticks
 *
 * return: 0 on success, negtive value on error
 */

int mq_timedsend (mq_id mq, void * buff, size_t size, unsigned int timeout)
    {
    return __mq_transfer (mq, buff, size, MQ_OP_WT, timeout);
    }

/**
 * mq_recv - receive a message from a message queue
 * @mq:   the message queue
 * @buff: the receive buffer
 * @size: the buffer size
 *
 * return: 0 on success, negtive value on error
 */

int mq_recv (mq_id mq, void * buff, size_t size)
    {
    return __mq_transfer (mq, buff, size, MQ_OP_RD, UINT_MAX);
    }

/**
 * mq_timedrecv - receive a message from a message queue with timeout
 * @mq:   the message queue
 * @buff: the receive buffer
 * @size: the buffer size
 * @timeout: the max number of waiting ticks
 *
 * return: 0 on success, negtive value on error
 */

int mq_timedrecv (mq_id mq, void * buff, size_t size, unsigned int timeout)
    {
    return __mq_transfer (mq, buff, size, MQ_OP_RD, timeout);
    }

