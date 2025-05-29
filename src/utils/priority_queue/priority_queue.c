#include "priority_queue.h"
#include "utils/utils.h"
#include <stdlib.h>

/*
二叉堆中的元素可以存储在数组中
数组形式：[8][6][5][1][2][][]
二叉堆形式：
             indx0:[8]
   indx1:[6]            indx2:[5]
indx3:[1]  indx4:[2]  indx5:[]   indx6:[]
left_child = father * 2 + 1
righ_child = father * 2 + 2
 */

pq_t *pq_init(int32_t capacity, int32_t (*compare)(key_value_t* A, key_value_t *b))
{
    if (NULL == compare)
    {
        d_log("compare func is NULL");
        return NULL;
    }
    pq_t *pq = (pq_t *)malloc(sizeof(pq_t));
    if (NULL == pq)
    {
        return NULL;
    }

    pq->capacity = capacity;
    pq->elements = (key_value_t *)malloc(sizeof(key_value_t)*pq->capacity);
    if (NULL == pq->elements)
    {
        free(pq);
        return NULL;
    }
    pq->compare = compare;
    return pq;
}

static void swap_elements(key_value_t* father, key_value_t *chil)
{
    key_value_t tmp = *father;
    *father = *chil;
    *chil = tmp;
}

int32_t max_heap_compare(key_value_t *a, key_value_t *b)
{
    return a->_key - b->_key;
}


int32_t min_heap_compare(key_value_t *a, key_value_t *b)
{
    return b->_key - a->_key;
}

bool priority_queue_push(pq_t *pq, key_value_t item)
{
    if (NULL == pq || NULL == pq->elements)
    {
        d_log("Invalid queue");
        return false;
    }

    if (pq->size >= pq->capacity)
    {
        d_log("Queue is full");
        return false;
    }
    // 插入新元素到数组末尾
    int32_t index = pq->size;
    pq->elements[index] = item;
    pq->size++;

    while (index > 0)
    {
        int32_t parent_index = (index - 1) / 2;
        key_value_t *parent = &pq->elements[parent_index];
        key_value_t *current = &pq->elements[index];

        if (pq->compare(parent, current) >= 0)
        {
            break;
        }
        swap_elements(parent, current);
        index = parent_index;
    }

    return true;
}

key_value_t pq_top(pq_t *pq)
{
    if (pq == NULL || pq->size == 0)
    {
        d_log("Queue is empty or invalid");
        key_value_t empty = {0, NULL};
        return empty;
    }
    return pq->elements[0];
}

bool priority_queue_pop(pq_t *pq)
{
    if (NULL == pq || 0 >= pq->size)
    {
        return false;
    }

    pq->size--;
    pq->elements[0] = pq->elements[pq->size];
    int32_t father = 0;
    while (true)
    {
        int32_t left_child = 2 * father + 1;
        int32_t right_child = 2 * father + 2;

        int32_t largest  = father;
        if (left_child < pq->size &&
            pq->compare(&pq->elements[left_child], &pq->elements[largest]) > 0)
        {
            largest = left_child;
        }
        if (right_child < pq->size &&
            pq->compare(&pq->elements[right_child], &pq->elements[largest]) > 0)
        {
            largest = right_child;
        }
        if (largest == father)
        {
            break; // 当前节点大于等于子节点，堆性质已恢复
        }
        // 否则交换当前节点和较大的子节点
        swap_elements(&pq->elements[father], &pq->elements[largest]);
        father = largest;
    }
    return true;
}

bool pq_empty(pq_t *pq)
{
    if (NULL == pq) {
        d_log("Invalid queue pointer");
        return true;
    }
    return pq->size == 0;
}
