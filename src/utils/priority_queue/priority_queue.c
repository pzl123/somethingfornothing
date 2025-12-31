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

#define AGING_TIME (1000U)

pq_t *pq_init(int32_t capacity, int32_t (*compare)(pv_t *A, pv_t *b))
{
    if (NULL == compare)
    {
        e_log("compare func is NULL");
        return NULL;
    }
    pq_t *pq = (pq_t *)malloc(sizeof(pq_t));
    if (NULL == pq)
    {
        return NULL;
    }

    pq->capacity = capacity;
    pq->elements = (pv_t *)malloc(sizeof(pv_t) * pq->capacity);
    if (NULL == pq->elements)
    {
        free(pq);
        return NULL;
    }
    pq->compare = compare;
    pq->size = 0;
    (void)pthread_mutex_init(&pq->mutex, NULL);
    (void)pthread_cond_init(&pq->cond, NULL);
    return pq;
}

bool pq_delete(pq_t *pq)
{
    if (pq == NULL)
    {
        return true;
    }

    pthread_mutex_destroy(&pq->mutex);
    pthread_cond_destroy(&pq->cond);

    if (pq->elements != NULL)
    {
        free(pq->elements);
    }
    free(pq);
    return true;
}

static void swap_elements(pv_t *father, pv_t *chil)
{
    pv_t tmp = *father;
    *father = *chil;
    *chil = tmp;
}

int32_t aging_compare(pv_t *a, pv_t *b)
{
    uint64_t now = gettime_msec();
    int age_a = (now - a->timestamp) / AGING_TIME; // 每秒提升1级
    int age_b = (now - b->timestamp) / AGING_TIME;

    int eff_a = a->_priority + age_a;
    int eff_b = b->_priority + age_b;

    return eff_b - eff_a; // max-heap: 高有效优先级在前
}

int32_t max_heap_compare(pv_t *a, pv_t *b)
{
    return a->_priority - b->_priority;
}

int32_t min_heap_compare(pv_t *a, pv_t *b)
{
    return b->_priority - a->_priority;
}

bool priority_queue_push(pq_t *pq, pv_t item)
{
    if (NULL == pq || NULL == pq->elements)
    {
        e_log("Invalid queue");
        return false;
    }

    pthread_mutex_lock(&pq->mutex);
    if (pq_full(pq))
    {
        // log_w("queue is full, capacity:%d size:%d, pq util:%f", pq->capacity, pq->size, (float)(((float)pq->size)/((float)pq->capacity)));
        pthread_mutex_unlock(&pq->mutex);
        return false;
    }

    // 插入新元素到数组末尾
    int32_t index = pq->size;
    pq->elements[index] = item;
    pq->size++;

    while (index > 0)
    {
        int32_t parent_index = (index - 1) / 2;
        pv_t *parent = &pq->elements[parent_index];
        pv_t *current = &pq->elements[index];

        if (pq->compare(parent, current) >= 0)
        {
            break;
        }
        swap_elements(parent, current);
        index = parent_index;
    }
    pthread_cond_signal(&pq->cond);
    pthread_mutex_unlock(&pq->mutex);
    return true;
}

pv_t pq_top(pq_t *pq)
{
    if (pq == NULL || pq->size == 0)
    {
        e_log("Queue is empty or invalid");
        pv_t empty = {0, NULL, 0};
        return empty;
    }
    return pq->elements[0];
}

bool priority_queue_try_pop(pq_t *pq, pv_t *item)
{
    if (NULL == pq || NULL == item)
    {
        return false;
    }

    pthread_mutex_lock(&pq->mutex);

    if (pq_empty(pq))
    {
        pthread_mutex_unlock(&pq->mutex);
        return false;
    }

    *item = pq->elements[0];
    pq->size--;
    pq->elements[0] = pq->elements[pq->size];
    int32_t father = 0;
    while (true)
    {
        int32_t left_child = 2 * father + 1;
        int32_t right_child = 2 * father + 2;

        int32_t largest_index = father;
        if (left_child < pq->size &&
            pq->compare(&pq->elements[left_child], &pq->elements[largest_index]) > 0)
        {
            largest_index = left_child;
        }
        if (right_child < pq->size &&
            pq->compare(&pq->elements[right_child], &pq->elements[largest_index]) > 0)
        {
            largest_index = right_child;
        }
        if (largest_index == father)
        {
            break;
        }
        swap_elements(&pq->elements[father], &pq->elements[largest_index]);
        father = largest_index;
    }
    pthread_mutex_unlock(&pq->mutex);
    return true;
}

bool priority_queue_pop(pq_t *pq, pv_t *item)
{
    if (NULL == pq || NULL == item)
    {
        return false;
    }

    pthread_mutex_lock(&pq->mutex);

    while (pq_empty(pq))
    {
        pthread_cond_wait(&pq->cond, &pq->mutex);
    }

    *item = pq->elements[0];
    pq->size--;
    pq->elements[0] = pq->elements[pq->size];
    int32_t father = 0;
    while (true)
    {
        int32_t left_child = 2 * father + 1;
        int32_t right_child = 2 * father + 2;

        int32_t largest_index = father;
        if (left_child < pq->size &&
            pq->compare(&pq->elements[left_child], &pq->elements[largest_index]) > 0)
        {
            largest_index = left_child;
        }
        if (right_child < pq->size &&
            pq->compare(&pq->elements[right_child], &pq->elements[largest_index]) > 0)
        {
            largest_index = right_child;
        }
        if (largest_index == father)
        {
            break; // 当前节点大于等于子节点，堆性质已恢复
        }
        // 否则交换当前节点和较大的子节点
        swap_elements(&pq->elements[father], &pq->elements[largest_index]);
        father = largest_index;
    }
    pthread_mutex_unlock(&pq->mutex);
    return true;
}

bool pq_full(pq_t *pq)
{
    if (NULL == pq)
    {
        e_log("Invalid queue pointer");
        return true;
    }
    return pq->size >= pq->capacity;
}

bool pq_empty(pq_t *pq)
{
    if (NULL == pq)
    {
        e_log("Invalid queue pointer");
        return true;
    }
    return pq->size == 0;
}

bool priority_queue_clear(pq_t *pq, void (*free_callback)(void*))
{
    if (NULL == pq)
    {
        e_log("Invalid queue pointer");
        return false;
    }

    pthread_mutex_lock(&pq->mutex);
    if (free_callback != NULL)
    {
        for (int i = 0; i < pq->size; i++)
        {
            if (pq->elements[i]._value != NULL)
            {
                free_callback(pq->elements[i]._value);
                pq->elements[i]._value = NULL;
            }
        }
    }
    pq->size = 0;
    pthread_mutex_unlock(&pq->mutex);
    return true;
}
