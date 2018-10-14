/* avltree.h - avl tree header file */

/*
 * Copyright (c) 2012 Fangming Chai
 */

/*
modification history
--------------------
01a,01oct18,cfm  writen
*/

#ifndef __AVLTREE_H__
#define __AVLTREE_H__

#include "__binary_tree.h"

/* typedefs */

typedef struct avl_node
    {
    bi_node_t  bin;
    int        height;
    } avl_node_t;

typedef struct avl_tree
    {
    bi_tree_t  bit;
    } avl_tree_t;

#endif  /* __AVLTREE_H__ */

