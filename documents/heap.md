介绍一种基于平衡二叉树实现的bestfit内存管理实现

实现目标：

1. 支持多块不连续内存的管理
2. 通过平衡二叉树降低寻找最佳内存块的时间复杂度

# 总体实现概述

## heap结构
内存管理的总结构，可以为每种不同类型的RAM分配一个，比如SRAM，DRAM，uncached内存。然后系统malloc可以默认使用一个。或者简单起见SRAM，DRAM都放在一个heap里。heap有能力管理不连续的多块内存。
1. heap结构管理所有的大块内存，可以管理任意个数的不连续的内存块。所有的块以链表的形式链接在heap结构中，但是在具体的内存分配中并不使用这个链表，这个链表仅仅维护加入heap的内存块信息。
2. heap结构中维护一个名为`size_tree`的平衡二叉树，比如红黑树
3. heap包含一个互斥锁供互斥访问heap
4. 额外的`size_nodes[n]`下面会讨论到
5. 内存分配信息`mem_stat`

## block结构
这里把每一块连续的内存块称作`block`。每一块连续的`block`可以被加入到某个heap管理。这个结构在实现中的重要性比较低。

## chunk结构
block被划分成多块chunk，chunk的数量会随着动态内存分配和是否增长下降。每一块被分配的，可以被分配的（free的）内存都是一个chunk。chunk是管理的基本单位。
每个chunk的开头被称作CHUNK_HEAD，CHUNK_HEAD又分为
1. ALLOCATED_CHUNK_HEAD，这里简称ACH
2. FREE_CHUNK_HEAD，这里简称FCH。
ACH和FCH位于每个chunk的开头，并通过union的方式公用空间，通过内部信息可以确定一个chunk是FCH还是ACH。FCH和ACH相同的成员是`prev_size`和`size`分别记录前一个chunk和当前chunk的大小。这样整个block中的所有chunk就通过prev size和size很自然的形成一个链表：
* next_chunk = (chunk*)((char *) chunk + chunk->size);
* prev_chunk = (chunk*)((char *) chunk - chunk->prev_size);
每个block在加入heap中的时候都被分成3个chunk，开头结尾各一个ACH作为哨兵，中间一个FCH
|0|1|2|
|-|-|-|
|ACH|FCH|ACH|
每个chunk都被加入heap中size_tree中跟自己大小相等键值的node下，也就是说所有block中的所有大小为S的chunk都被加入到heap的size_tree中键值为S的节点下。

## size_node
size_node用来管理每种不同大小的free chunk，所有block中相同大小的chunk都被链接在同一个size_node下。然后size_node被插入到平衡二叉树中。
### size_node的分配
因为size_node本身需要内存空间，所以它的分配就显得相当重要，这里采取两种方式分配size_node:
1. 因为size_node本身管理的就是chunk，而chunk本身就包含有一块内存，那么如果chunk中管理的内存大小刚好大于等于size_node的话就借用当前chunk中的内存存放size_node。显然每个大小第一个被放入heap的chunk中的内存会被用作size_node。
2. 如果chunk中的内存小于size_node，那么就是从heap中的`size_node`中分配。这种情况需要重点探讨一下。首先需要理清几个事实：
  * 内存的分配不是任意大小的，事实上内存分配都是需要对齐到一个默认对齐值的。
  * size_node的大小是固定的
  * chunk管理的内存大小都是大于0，也就是说chunk的大小大于sizeof (FCH)
  * chunk本身以及chunk中管理的内存的大小都是分配对齐值的倍数（通过实现限定）
  所以:
  1. chunk->size = n * ALLOC_ALIGN
  2. chunk->size > sizeof (struct chunk)
  因此：
  heap->size_nodes[0]将会被用于chunk->size == sizeof (struct chunk) + ALLOC_ALIGN的chunk
  heap->size_nodes[1]将会被用于chunk->size == sizeof (struct chunk) + ALLOC_ALIGN * 2的chunk
  所以在此数组中的idx为
  idx = (chunk->size - sizeof (struct chunk)) / ALLOC_ALIGN - 1;
  因为只有chunk->size小于sizeof (size_node)的时候才会使用这个数组，所以使用这个数组的chunk的最大大小为：
    ```C
    max_size = sizeof (struct chunk) + sizeof (size_node) - ALLOC_ALIGN;
    ```
    所以最大idx为：
    ```C
    max = (max_size - sizeof (struct chunk)) / ALLOC_ALIGN - 1;
    ```
    -->
    ```C
    max = (sizeof (struct chunk) + sizeof (size_node) - ALLOC_ALIGN - sizeof (struct chunk)) / ALLOC_ALIGN - 1;
    ```
    -->
    ```C
    max = (sizeof (size_node) - ALLOC_ALIGN) / ALLOC_ALIGN - 1;
    ```
    所以heap->size_nodes[n]中的n应该为：
    ```C
    n = (sizeof (size_node) - ALLOC_ALIGN) / ALLOC_ALIGN;
    ```
    等同于
    ```C
    n = (sizeof (size_node) - 1) / ALLOC_ALIGN;
    ```
