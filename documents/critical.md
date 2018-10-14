"critical"模块是本RTOS的核心所在。

# 避免关中断的重要性

这里我们先回顾一下RTOS最重要的特性：

1. 及时的中断响应
2. 及时的高优先级任务保证

中断往往发生在有外部事件急需处理的情况。而且当一个较低优先级任务正在执行的时候，高优先级任务通常由当前任务通过IPC或由外部中断唤醒。所以，中断的响应明显更重要。而大多嵌入式OS（此处没有使用RTOS，而是使用嵌入式OS，因为二者其实并不等同，RTOS通常都是嵌入式OS，但嵌入式OS不一定是RTOS）在操作一些核心数据结构的时候通常会关闭中断，而更有甚者，有些嵌入式OS几乎在大多数核心函数的开头关闭中断，在函数退出时打开中断，这显然有悖于RTOS对中断响应的要求。如果一个嵌入式OS仅仅实现基于优先级的任务调度，而对中断关闭的规避考虑不够的话，其实并不能称作真正的RTOS。而本模块就是为了解决尽量减少关闭中断访问核心数据（通常是任务管理相关结构）的问题，换言之就是尽量减少中断关闭的窗口的情况下与中断互斥访问核心数据。

基于中断的作用，中断与任务，以及低优先级中断与高优先级中断最关键的互斥数据就是任务的相关操作了。比如ready队列的操作，各种IPC资源的pend队列操作，超时队列的操作，等等。

# 本OS中的实现

## 实现概述
"critical"模块跟中断互斥的机制十分简单，无论在中断模式还是在非中断模式的任务里，进行互斥操作前将一个变量简单的设置一下就可以了，这个变量就是：
```C
volatile bool in_critical = false;
```
如何保证跟中断的互斥：
1. 进行关键资源互斥操作前，先判断`in_critical`是否为`true`
2. 如果是真，就把要做的工作加入的到一个名为`critical_job`的队列中。在加入到队列的时候只须关闭很短时间的中断
3. 如果`in_critical`为`false`那么就简单的将其设置为`true`然后调用需要处理的函数
4. 处理完自己的函数后需要检查`critical_job`是不是空，如果不是空，那么就清空这个队列，清空时，调用队列中的每一个回调函数，在此处处理队列的时候也只需关闭很短时间的中断。
5. 判断是否需要任务调度时，如果`in_critical`为`true`那么判定为不调度
6. 退出"critical"状态后，需要检查是否需要任务调度，这样在"critical"里任何变为"ready"的高优先级任务都可以得到及时的执行
7. 避免嵌套，在需要进入"critical"状态的函数中，不要调用其他需要进入"critical"状态的函数
8. 回调函数应尽量精简
9. 本OS目前只考虑MCU，故暂不考虑SMP的情况

那么， 只有在中断上下文才有可能添加回调函数到`critical_job`的队列中。

## 接口支持
本OS提供了一组函数供内核使用：

1. `do_critical`，调用此函数时，传入的回调函数必须不能导致睡眠，因为依赖此函数实现的是可以在中断上下文调用的，比如`task_resume`，`event_send`，`sem_post`，等等。形如以下代码片段：
    ```C
    int task_resume (task_id task)
        {

        /* resuming current task, current task is running */

        if (task == NULL)
            {
            return 0;
            }

        return do_critical (__task_resume, (uintptr_t) task, 0);
        }
    ```
2. `do_critical_might_sleep`，传入此函数的回调函数有可能导致当前任务睡眠，也就是可能会操作ready队列，并从中删除当前任务，那样在推出`critical`的时候就会触发任务调度。基于此接口实现的函数不支持在锁任务调度，中断上下文以及idle上下文使用。比如`task_delay`，`sem_wait`，`mutex_lock`等等。形如以下代码片段：
    ```C
    int sem_wait (sem_t * sem)
        {
        return do_critical_might_sleep (__sem_wait, (uintptr_t) sem, UINT_MAX);
        }
    ```
    `do_critical_might_sleep`中已经包含了调用限制检查，不需要每一个有这种限制的接口自己检查：
    ```C
    int do_critical_might_sleep (int (* job) (uintptr_t, uintptr_t),
                                 uintptr_t arg1, uintptr_t arg2)
        {
        if (g_task_lock_cnt || (g_int_cnt > 0) || (current == idle))
            {
            return -1;
            }

        return __do_critical (job, arg1, arg2);
        }
    ```
3. `do_critical_non_irq`，调用此接口的时候传入的回调函数不会导致睡眠，但是从逻辑上限制不能在中断上下文调用。一般使用在函数必须同步完成，不能被延迟调用（加入到`critical_job`队列中。但是如果不限制在中断上下文调用就需要额外的机制保证互斥操作。属于此类的接口包括：`sem_trywait`，`mutex_trylock`等等。如果诸如`sem_wait`之类的接口在任务上下文正在操作一个信号量，正在此时，被一个中断打断，假如允许中断中使用`sem_trywait`，那么因为这个函数不能delay执行，调用者立刻需要执行结果，而且不能产生任务调度。所以要保证正确性的话需要额外的机制（比如关中断）保证互斥性。为了避免此类情况，在逻辑上额外限制不可以在中断上下文调用此类函数，从而避免了这种情况。而且在中断中禁止此类调用没有太多功能上的损失。形如以下代码片段：
    ```C
    int sem_trywait (sem_t * sem)
        {
        return do_critical_non_irq (__sem_wait, (uintptr_t) sem, 0);
        }
    ```
    `do_critical_non_irq`中已经包含了调用限制检查，不需要每一个有这种限制的接口自己检查：
    ```C
    int do_critical_non_irq (int (* job) (uintptr_t, uintptr_t),
                             uintptr_t arg1, uintptr_t arg2)
        {
        if (g_int_cnt > 0)
            {
            return -1;
            }

        return __do_critical (job, arg1, arg2);
        }
    ```